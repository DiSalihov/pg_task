#include "include.h"

static volatile sig_atomic_t sigterm = false;

void task_work(Task *task) {
    #define ID 1
    #define SID S(ID)
    #define PID 2
    #define SPID S(PID)
    Work *work = task->work;
    static Oid argtypes[] = {[ID - 1] = INT8OID, [PID - 1] = INT4OID};
    Datum values[] = {[ID - 1] = Int64GetDatum(task->id), [PID - 1] = Int32GetDatum(task->pid)};
    static SPIPlanPtr plan = NULL;
    static char *command = NULL;
    StaticAssertStmt(sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0]), "sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0])");
    if (!task->remote) {
        StringInfoData buf;
        initStringInfo(&buf);
        appendStringInfo(&buf, "%lu", task->id);
        SetConfigOptionMy("pg_task.id", buf.data);
        pfree(buf.data);
    }
    task->count++;
    if (!command) {
        StringInfoData buf;
        initStringInfo(&buf);
        appendStringInfo(&buf,
            "WITH s AS (SELECT id FROM %1$s WHERE id = $" SID " FOR UPDATE)\n"
            "UPDATE  %1$s AS u\n"
            "SET     state = 'WORK'::state,\n"
            "        start = current_timestamp,\n"
            "        pid = $" SPID "\n"
            "FROM s WHERE u.id = s.id RETURNING request, COALESCE(EXTRACT(epoch FROM timeout), 0)::int4 * 1000 AS timeout", work->schema_table);
        command = buf.data;
    }
    #undef ID
    #undef SID
    #undef PID
    #undef SPID
    SPI_connect_my(command);
    if (!plan) plan = SPI_prepare_my(command, sizeof(argtypes)/sizeof(argtypes[0]), argtypes);
    SPI_execute_plan_my(plan, values, NULL, SPI_OK_UPDATE_RETURNING);
    if (SPI_processed != 1) E("SPI_processed != 1"); else {
        MemoryContext oldMemoryContext = MemoryContextSwitchTo(work->context);
        bool timeout_isnull;
        task->request = SPI_getvalue_my(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, SPI_fnumber(SPI_tuptable->tupdesc, "request"));
        task->timeout = DatumGetInt32(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, SPI_fnumber(SPI_tuptable->tupdesc, "timeout"), &timeout_isnull));
        if (0 < StatementTimeout && StatementTimeout < task->timeout) task->timeout = StatementTimeout;
        L("request = %s, timeout = %i", task->request, task->timeout);
        if (timeout_isnull) E("timeout_isnull");
        MemoryContextSwitchTo(oldMemoryContext);
    }
    SPI_commit_my(command);
    SPI_finish_my(command);
}

static void task_repeat(Task *task) {
    #define ID 1
    #define SID S(ID)
    static Oid argtypes[] = {[ID - 1] = INT8OID};
    Datum values[] = {[ID - 1] = Int64GetDatum(task->id)};
    static SPIPlanPtr plan = NULL;
    static char *command = NULL;
    StaticAssertStmt(sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0]), "sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0])");
    if (!command) {
        Work *work = task->work;
        StringInfoData buf;
        initStringInfo(&buf);
        appendStringInfo(&buf,
            "INSERT INTO %1$s (dt, queue, max, request, timeout, delete, repeat, drift, count, live)\n"
            "SELECT CASE WHEN drift THEN current_timestamp + repeat\n"
            "ELSE (WITH RECURSIVE s AS (SELECT dt AS t UNION SELECT t + repeat FROM s WHERE t <= current_timestamp) SELECT * FROM s ORDER BY 1 DESC LIMIT 1)\n"
            "END AS dt, queue, max, request, timeout, delete, repeat, drift, count, live\n"
            "FROM %1$s WHERE id = $" SID " AND state IN ('DONE'::state, 'FAIL'::state) LIMIT 1", work->schema_table);
        command = buf.data;
    }
    #undef ID
    #undef SID
    SPI_connect_my(command);
    if (!plan) plan = SPI_prepare_my(command, sizeof(argtypes)/sizeof(argtypes[0]), argtypes);
    SPI_execute_plan_my(plan, values, NULL, SPI_OK_INSERT);
    SPI_commit_my(command);
    SPI_finish_my(command);
}

static void task_delete(Task *task) {
    #define ID 1
    #define SID S(ID)
    static Oid argtypes[] = {[ID - 1] = INT8OID};
    Datum values[] = {[ID - 1] = Int64GetDatum(task->id)};
    static SPIPlanPtr plan = NULL;
    static char *command = NULL;
    StaticAssertStmt(sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0]), "sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0])");
    if (!command) {
        Work *work = task->work;
        StringInfoData buf;
        initStringInfo(&buf);
        appendStringInfo(&buf, "DELETE FROM %s WHERE id = $" SID, work->schema_table);
        command = buf.data;
    }
    #undef ID
    #undef SID
    SPI_connect_my(command);
    if (!plan) plan = SPI_prepare_my(command, sizeof(argtypes)/sizeof(argtypes[0]), argtypes);
    SPI_execute_plan_my(plan, values, NULL, SPI_OK_DELETE);
    SPI_commit_my(command);
    SPI_finish_my(command);
}

static bool task_live(Task *task) {
    #define QUEUE 1
    #define SQUEUE S(QUEUE)
    #define MAX 2
    #define SMAX S(MAX)
    #define COUNT 3
    #define SCOUNT S(COUNT)
    #define START 4
    #define SSTART S(START)
    bool exit = false;
    static Oid argtypes[] = {[QUEUE - 1] = TEXTOID, [MAX - 1] = INT4OID, [COUNT - 1] = INT4OID, [START - 1] = TIMESTAMPTZOID};
    Datum values[] = {[QUEUE - 1] = CStringGetTextDatum(task->queue), [MAX - 1] = Int32GetDatum(task->max), [COUNT - 1] = Int32GetDatum(task->count), [START - 1] = TimestampTzGetDatum(task->start)};
    static SPIPlanPtr plan = NULL;
    static char *command = NULL;
    StaticAssertStmt(sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0]), "sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0])");
    if (!command) {
        Work *work = task->work;
        StringInfoData buf;
        initStringInfo(&buf);
        appendStringInfo(&buf,
            "WITH s AS (\n"
            "SELECT  id\n"
            "FROM    %1$s\n"
            "WHERE   state = 'PLAN'::state\n"
            "AND     dt <= current_timestamp\n"
            "AND     queue = $" SQUEUE "\n"
            "AND     COALESCE(max, ~(1<<31)) >= $" SMAX "\n"
            "AND     CASE WHEN count IS NOT NULL AND live IS NOT NULL THEN count > $" SCOUNT " AND $" SSTART " + live > current_timestamp ELSE COALESCE(count, 0) > $" SCOUNT " OR $" SSTART " + COALESCE(live, '0 sec'::interval) > current_timestamp END\n"
            "ORDER BY COALESCE(max, ~(1<<31)) DESC LIMIT 1 FOR UPDATE SKIP LOCKED\n"
            ") UPDATE %1$s AS u SET state = 'TAKE'::state FROM s WHERE u.id = s.id RETURNING u.id", work->schema_table);
        command = buf.data;
    }
    #undef MAX
    #undef SMAX
    #undef COUNT
    #undef SCOUNT
    #undef START
    #undef SSTART
    SPI_connect_my(command);
    if (!plan) plan = SPI_prepare_my(command, sizeof(argtypes)/sizeof(argtypes[0]), argtypes);
    SPI_execute_plan_my(plan, values, NULL, SPI_OK_UPDATE_RETURNING);
    if (!SPI_processed) exit = true; else {
        bool id_isnull;
        task->id = DatumGetInt64(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, SPI_fnumber(SPI_tuptable->tupdesc, "id"), &id_isnull));
        if (id_isnull) E("id_isnull");
    }
    SPI_commit_my(command);
    SPI_finish_my(command);
    pfree((void *)values[QUEUE - 1]);
    #undef QUEUE
    #undef SQUEUE
    return exit;
}

void task_done(Task *task) {
    #define ID 1
    #define SID S(ID)
    #define SUCCESS 2
    #define SSUCCESS S(SUCCESS)
    #define RESPONSE 3
    #define SRESPONSE S(RESPONSE)
    static Oid argtypes[] = {[ID - 1] = INT8OID, [SUCCESS - 1] = BOOLOID, [RESPONSE - 1] = TEXTOID};
    Datum values[] = {[ID - 1] = Int64GetDatum(task->id), [SUCCESS - 1] = BoolGetDatum(task->success), [RESPONSE - 1] = task->response.data ? CStringGetTextDatum(task->response.data) : (Datum)NULL};
    char nulls[] = {[ID - 1] = ' ', [SUCCESS - 1] = ' ', [RESPONSE - 1] = task->response.data ? ' ' : 'n'};
    static SPIPlanPtr plan = NULL;
    static char *command = NULL;
    StaticAssertStmt(sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0]), "sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0])");
    StaticAssertStmt(sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(nulls)/sizeof(nulls[0]), "sizeof(argtypes)/sizeof(argtypes[0]) == sizeof(values)/sizeof(values[0])");
    L("id = %lu, response = %s, success = %s", task->id, task->response.data ? task->response.data : "(null)", task->success ? "true" : "false");
    if (!command) {
        Work *work = task->work;
        StringInfoData buf;
        initStringInfo(&buf);
        appendStringInfo(&buf,
            "WITH s AS (SELECT id FROM %1$s WHERE id = $" SID " FOR UPDATE\n)\n"
            "UPDATE %1$s AS u SET state = CASE WHEN $" SSUCCESS " THEN 'DONE'::state ELSE 'FAIL'::state END, stop = current_timestamp, response = $" SRESPONSE " FROM s WHERE u.id = s.id\n"
            "RETURNING delete, repeat IS NOT NULL AND state IN ('DONE'::state, 'FAIL'::state) AS repeat, count IS NOT NULL OR live IS NOT NULL AS live", work->schema_table);
        command = buf.data;
    }
    #undef ID
    #undef SID
    #undef SUCCESS
    #undef SSUCCESS
    L("IsTransactionState = %s, IsAbortedTransactionBlockState = %s", IsTransactionState() ? "true" : "false", IsAbortedTransactionBlockState() ? "true": "false");
    SPI_connect_my(command);
    L("IsTransactionState = %s, IsAbortedTransactionBlockState = %s", IsTransactionState() ? "true" : "false", IsAbortedTransactionBlockState() ? "true": "false");
    if (!plan) plan = SPI_prepare_my(command, sizeof(argtypes)/sizeof(argtypes[0]), argtypes);
    L("hi3");
    SPI_execute_plan_my(plan, values, nulls, SPI_OK_UPDATE_RETURNING);
    L("hi4");
    if (SPI_processed != 1) E("SPI_processed != 1"); else {
        bool delete_isnull, repeat_isnull, live_isnull;
        task->delete = DatumGetBool(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, SPI_fnumber(SPI_tuptable->tupdesc, "delete"), &delete_isnull));
        task->repeat = DatumGetBool(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, SPI_fnumber(SPI_tuptable->tupdesc, "repeat"), &repeat_isnull));
        task->live = DatumGetBool(SPI_getbinval(SPI_tuptable->vals[0], SPI_tuptable->tupdesc, SPI_fnumber(SPI_tuptable->tupdesc, "live"), &live_isnull));
        if (delete_isnull) E("delete_isnull");
        if (repeat_isnull) E("repeat_isnull");
        if (live_isnull) E("live_isnull");
    }
    SPI_commit_my(command);
    SPI_finish_my(command);
    if (task->response.data) pfree((void *)values[RESPONSE - 1]);
    #undef RESPONSE
    #undef SRESPONSE
}

static void task_success(Task *task) {
    MemoryContext oldMemoryContext = MemoryContextSwitchTo(MessageContext);
    MemoryContextResetAndDeleteChildren(MessageContext);
    InvalidateCatalogSnapshotConditionally();
    MemoryContextSwitchTo(oldMemoryContext);
    SetCurrentStatementStartTimestamp();
    exec_simple_query(task);
    pgstat_report_stat(false);
    pgstat_report_activity(STATE_IDLE, NULL);
}

static void task_error(Task *task) {
    Work *work = task->work;
    MemoryContext oldMemoryContext = MemoryContextSwitchTo(work->context);
    ErrorData *edata = CopyErrorData();
    initStringInfo(&task->response);
    appendStringInfo(&task->response, "elevel::int4\t%i", edata->elevel);
    if (edata->output_to_server) appendStringInfoString(&task->response, "\noutput_to_server::bool\ttrue");
    if (edata->output_to_client) appendStringInfoString(&task->response, "\noutput_to_client::bool\ttrue");
    if (edata->show_funcname) appendStringInfoString(&task->response, "\nshow_funcname::bool\ttrue");
    if (edata->hide_stmt) appendStringInfoString(&task->response, "\nhide_stmt::bool\ttrue");
    if (edata->hide_ctx) appendStringInfoString(&task->response, "\nhide_ctx::bool\ttrue");
    if (edata->filename) appendStringInfo(&task->response, "\nfilename::text\t%s", edata->filename);
    if (edata->lineno) appendStringInfo(&task->response, "\nlineno::int4\t%i", edata->lineno);
    if (edata->funcname) appendStringInfo(&task->response, "\nfuncname::text\t%s", edata->funcname);
    if (edata->domain) appendStringInfo(&task->response, "\ndomain::text\t%s", edata->domain);
    if (edata->context_domain) appendStringInfo(&task->response, "\ncontext_domain::text\t%s", edata->context_domain);
    if (edata->sqlerrcode) appendStringInfo(&task->response, "\nsqlerrcode::int4\t%i", edata->sqlerrcode);
    if (edata->message) appendStringInfo(&task->response, "\nmessage::text\t%s", edata->message);
    if (edata->detail) appendStringInfo(&task->response, "\ndetail::text\t%s", edata->detail);
    if (edata->detail_log) appendStringInfo(&task->response, "\ndetail_log::text\t%s", edata->detail_log);
    if (edata->hint) appendStringInfo(&task->response, "\nhint::text\t%s", edata->hint);
    if (edata->context) appendStringInfo(&task->response, "\ncontext::text\t%s", edata->context);
    if (edata->message_id) appendStringInfo(&task->response, "\nmessage_id::text\t%s", edata->message_id);
    if (edata->schema_name) appendStringInfo(&task->response, "\nschema_name::text\t%s", edata->schema_name);
    if (edata->table_name) appendStringInfo(&task->response, "\ntable_name::text\t%s", edata->table_name);
    if (edata->column_name) appendStringInfo(&task->response, "\ncolumn_name::text\t%s", edata->column_name);
    if (edata->datatype_name) appendStringInfo(&task->response, "\ndatatype_name::text\t%s", edata->datatype_name);
    if (edata->constraint_name) appendStringInfo(&task->response, "\nconstraint_name::text\t%s", edata->constraint_name);
    if (edata->cursorpos) appendStringInfo(&task->response, "\ncursorpos::int4\t%i", edata->cursorpos);
    if (edata->internalpos) appendStringInfo(&task->response, "\ninternalpos::int4\t%i", edata->internalpos);
    if (edata->internalquery) appendStringInfo(&task->response, "\ninternalquery::text\t%s", edata->internalquery);
    if (edata->saved_errno) appendStringInfo(&task->response, "\nsaved_errno::int4\t%i", edata->saved_errno);
    FreeErrorData(edata);
    MemoryContextSwitchTo(oldMemoryContext);
    SPI_rollback_my(task->request);
}

static bool task_loop(Task *task) {
    Work *work = task->work;
    if (!pg_try_advisory_lock_int4_my(work->oid, task->id)) E("lock id = %lu, oid = %d", task->id, work->oid);
    task_work(task);
    L("id = %lu, timeout = %d, request = %s, count = %u", task->id, task->timeout, task->request, task->count);
    L("PG_exception_stack = %p", PG_exception_stack);
    L("error_context_stack = %p", error_context_stack);
    PG_TRY();
        task_success(task);
    PG_CATCH();
        task_error(task);
    PG_END_TRY();
    pfree(task->request);
    task_done(task);
    L("repeat = %s, delete = %s, live = %s", task->repeat ? "true" : "false", task->delete ? "true" : "false", task->delete ? "true" : "false");
    if (task->repeat) task_repeat(task);
    if (task->delete && !task->response.data) task_delete(task);
    if (task->response.data) pfree(task->response.data);
    task->response.data = NULL;
    pg_advisory_unlock_int4_my(work->oid, task->id);
    return !task->live || task_live(task);
}

static void task_sigterm(SIGNAL_ARGS) {
    int save_errno = errno;
    sigterm = true;
    SetLatch(MyLatch);
    errno = save_errno;
}

static void task_init_conf(Conf *conf) {
    conf->p = MyBgworkerEntry->bgw_extra;
    conf->user = conf->p;
    conf->p += strlen(conf->user) + 1;
    conf->data = conf->p;
    conf->p += strlen(conf->data) + 1;
    conf->schema = conf->p;
    conf->p += strlen(conf->schema) + 1;
    conf->table = conf->p;
    conf->p += strlen(conf->table) + 1;
    if (conf->table == conf->schema + 1) conf->schema = NULL;
    if (!MessageContext) MessageContext = AllocSetContextCreate(TopMemoryContext, "MessageContext", ALLOCSET_DEFAULT_SIZES);
    if (!MyProcPort && !(MyProcPort = (Port *) calloc(1, sizeof(Port)))) E("!calloc");
    if (!MyProcPort->remote_host) MyProcPort->remote_host = "[local]";
    if (!MyProcPort->user_name) MyProcPort->user_name = conf->user;
    if (!MyProcPort->database_name) MyProcPort->database_name = conf->data;
    SetConfigOptionMy("application_name", MyBgworkerEntry->bgw_type);
    L("user = %s, data = %s, schema = %s, table = %s", conf->user, conf->data, conf->schema ? conf->schema : "(null)", conf->table);
    SetConfigOptionMy("pg_task.data", conf->data);
    SetConfigOptionMy("pg_task.user", conf->user);
    if (conf->schema) SetConfigOptionMy("pg_task.schema", conf->schema);
    SetConfigOptionMy("pg_task.table", conf->table);
}

static void task_init_work(Work *work) {
    StringInfoData buf;
    Conf *conf = &work->conf;
    const char *schema_quote = conf->schema ? quote_identifier(conf->schema) : NULL;
    const char *table_quote = quote_identifier(conf->table);
    initStringInfo(&buf);
    if (conf->schema) appendStringInfo(&buf, "%s.", schema_quote);
    appendStringInfoString(&buf, table_quote);
    work->schema_table = buf.data;
    work->oid = *(typeof(work->oid) *)conf->p;
    conf->p += sizeof(work->oid);
    L("oid = %d", work->oid);
    initStringInfo(&buf);
    appendStringInfo(&buf, "%d", work->oid);
    SetConfigOptionMy("pg_task.oid", buf.data);
    pfree(buf.data);
    if (!work->context) work->context = AllocSetContextCreate(TopMemoryContext, "myMemoryContext", ALLOCSET_DEFAULT_SIZES);
    if (conf->schema && schema_quote && conf->schema != schema_quote) pfree((void *)schema_quote);
    if (conf->table != table_quote) pfree((void *)table_quote);
}

static void task_init_task(Task *task) {
    Work *work = task->work;
    Conf *conf = &work->conf;
    task->pid = MyProcPid;
    task->id = MyBgworkerEntry->bgw_main_arg;
    task->start = GetCurrentTimestamp();
    task->count = 0;
    task->queue = conf->p;
    conf->p += strlen(task->queue) + 1;
    task->max = *(typeof(task->max) *)conf->p;
    conf->p += sizeof(task->max);
    L("id = %lu, queue = %s, max = %u", task->id, task->queue, task->max);
    pqsignal(SIGTERM, task_sigterm);
    BackgroundWorkerUnblockSignals();
    BackgroundWorkerInitializeConnection(conf->data, conf->user, 0);
    pgstat_report_appname(MyBgworkerEntry->bgw_type);
    SetConfigOptionMy("pg_task.queue", task->queue);
}

static void task_reset(void) {
    ResetLatch(MyLatch);
    CHECK_FOR_INTERRUPTS();
}

void task_worker(Datum main_arg); void task_worker(Datum main_arg) {
    Task task;
    Work work;
    MemSet(&task, 0, sizeof(task));
    MemSet(&work, 0, sizeof(work));
    task.work = &work;
    task_init_conf(&work.conf);
    task_init_work(&work);
    task_init_task(&task);
    while (!sigterm) {
        int rc = WaitLatch(MyLatch, WL_LATCH_SET | WL_TIMEOUT | WL_EXIT_ON_PM_DEATH, 0, PG_WAIT_EXTENSION);
        if (!BackendPidGetProc(MyBgworkerEntry->bgw_notify_pid)) break;
        if (rc & WL_LATCH_SET) task_reset();
        if (rc & WL_TIMEOUT) sigterm = task_loop(&task);
    }
}
