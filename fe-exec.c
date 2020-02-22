#include "include.h"

char *PQftypeMy(const PGresult *res, int column_number) {
    Oid oid = PQftype(res, column_number);
    StringInfoData buf;
    initStringInfo(&buf);
    switch (oid) {
        case BOOLOID: appendStringInfoString(&buf, "BOOL"); break;
        case BYTEAOID: appendStringInfoString(&buf, "BYTEA"); break;
        case CHAROID: appendStringInfoString(&buf, "CHAR"); break;
        case NAMEOID: appendStringInfoString(&buf, "NAME"); break;
        case INT8OID: appendStringInfoString(&buf, "INT8"); break;
        case INT2OID: appendStringInfoString(&buf, "INT2"); break;
        case INT2VECTOROID: appendStringInfoString(&buf, "INT2VECTOR"); break;
        case INT4OID: appendStringInfoString(&buf, "INT4"); break;
        case REGPROCOID: appendStringInfoString(&buf, "REGPROC"); break;
        case TEXTOID: appendStringInfoString(&buf, "TEXT"); break;
        case OIDOID: appendStringInfoString(&buf, "OID"); break;
        case TIDOID: appendStringInfoString(&buf, "TID"); break;
        case XIDOID: appendStringInfoString(&buf, "XID"); break;
        case CIDOID: appendStringInfoString(&buf, "CID"); break;
        case OIDVECTOROID: appendStringInfoString(&buf, "OIDVECTOR"); break;
        case JSONOID: appendStringInfoString(&buf, "JSON"); break;
        case XMLOID: appendStringInfoString(&buf, "XML"); break;
        case PGNODETREEOID: appendStringInfoString(&buf, "PGNODETREE"); break;
        case PGNDISTINCTOID: appendStringInfoString(&buf, "PGNDISTINCT"); break;
        case PGDEPENDENCIESOID: appendStringInfoString(&buf, "PGDEPENDENCIES"); break;
        case PGMCVLISTOID: appendStringInfoString(&buf, "PGMCVLIST"); break;
        case PGDDLCOMMANDOID: appendStringInfoString(&buf, "PGDDLCOMMAND"); break;
        case POINTOID: appendStringInfoString(&buf, "POINT"); break;
        case LSEGOID: appendStringInfoString(&buf, "LSEG"); break;
        case PATHOID: appendStringInfoString(&buf, "PATH"); break;
        case BOXOID: appendStringInfoString(&buf, "BOX"); break;
        case POLYGONOID: appendStringInfoString(&buf, "POLYGON"); break;
        case LINEOID: appendStringInfoString(&buf, "LINE"); break;
        case FLOAT4OID: appendStringInfoString(&buf, "FLOAT4"); break;
        case FLOAT8OID: appendStringInfoString(&buf, "FLOAT8"); break;
        case UNKNOWNOID: appendStringInfoString(&buf, "UNKNOWN"); break;
        case CIRCLEOID: appendStringInfoString(&buf, "CIRCLE"); break;
        case CASHOID: appendStringInfoString(&buf, "CASH"); break;
        case MACADDROID: appendStringInfoString(&buf, "MACADDR"); break;
        case INETOID: appendStringInfoString(&buf, "INET"); break;
        case CIDROID: appendStringInfoString(&buf, "CIDR"); break;
        case MACADDR8OID: appendStringInfoString(&buf, "MACADDR8"); break;
        case ACLITEMOID: appendStringInfoString(&buf, "ACLITEM"); break;
        case BPCHAROID: appendStringInfoString(&buf, "BPCHAR"); break;
        case VARCHAROID: appendStringInfoString(&buf, "VARCHAR"); break;
        case DATEOID: appendStringInfoString(&buf, "DATE"); break;
        case TIMEOID: appendStringInfoString(&buf, "TIME"); break;
        case TIMESTAMPOID: appendStringInfoString(&buf, "TIMESTAMP"); break;
        case TIMESTAMPTZOID: appendStringInfoString(&buf, "TIMESTAMPTZ"); break;
        case INTERVALOID: appendStringInfoString(&buf, "INTERVAL"); break;
        case TIMETZOID: appendStringInfoString(&buf, "TIMETZ"); break;
        case BITOID: appendStringInfoString(&buf, "BIT"); break;
        case VARBITOID: appendStringInfoString(&buf, "VARBIT"); break;
        case NUMERICOID: appendStringInfoString(&buf, "NUMERIC"); break;
        case REFCURSOROID: appendStringInfoString(&buf, "REFCURSOR"); break;
        case REGPROCEDUREOID: appendStringInfoString(&buf, "REGPROCEDURE"); break;
        case REGOPEROID: appendStringInfoString(&buf, "REGOPER"); break;
        case REGOPERATOROID: appendStringInfoString(&buf, "REGOPERATOR"); break;
        case REGCLASSOID: appendStringInfoString(&buf, "REGCLASS"); break;
        case REGTYPEOID: appendStringInfoString(&buf, "REGTYPE"); break;
        case REGROLEOID: appendStringInfoString(&buf, "REGROLE"); break;
        case REGNAMESPACEOID: appendStringInfoString(&buf, "REGNAMESPACE"); break;
        case UUIDOID: appendStringInfoString(&buf, "UUID"); break;
        case LSNOID: appendStringInfoString(&buf, "LSN"); break;
        case TSVECTOROID: appendStringInfoString(&buf, "TSVECTOR"); break;
        case GTSVECTOROID: appendStringInfoString(&buf, "GTSVECTOR"); break;
        case TSQUERYOID: appendStringInfoString(&buf, "TSQUERY"); break;
        case REGCONFIGOID: appendStringInfoString(&buf, "REGCONFIG"); break;
        case REGDICTIONARYOID: appendStringInfoString(&buf, "REGDICTIONARY"); break;
        case JSONBOID: appendStringInfoString(&buf, "JSONB"); break;
        case JSONPATHOID: appendStringInfoString(&buf, "JSONPATH"); break;
        case TXID_SNAPSHOTOID: appendStringInfoString(&buf, "TXID_SNAPSHOT"); break;
        case INT4RANGEOID: appendStringInfoString(&buf, "INT4RANGE"); break;
        case NUMRANGEOID: appendStringInfoString(&buf, "NUMRANGE"); break;
        case TSRANGEOID: appendStringInfoString(&buf, "TSRANGE"); break;
        case TSTZRANGEOID: appendStringInfoString(&buf, "TSTZRANGE"); break;
        case DATERANGEOID: appendStringInfoString(&buf, "DATERANGE"); break;
        case INT8RANGEOID: appendStringInfoString(&buf, "INT8RANGE"); break;
        case RECORDOID: appendStringInfoString(&buf, "RECORD"); break;
        case RECORDARRAYOID: appendStringInfoString(&buf, "RECORDARRAY"); break;
        case CSTRINGOID: appendStringInfoString(&buf, "CSTRING"); break;
        case ANYOID: appendStringInfoString(&buf, "ANY"); break;
        case ANYARRAYOID: appendStringInfoString(&buf, "ANYARRAY"); break;
        case VOIDOID: appendStringInfoString(&buf, "VOID"); break;
        case TRIGGEROID: appendStringInfoString(&buf, "TRIGGER"); break;
        case EVTTRIGGEROID: appendStringInfoString(&buf, "EVTTRIGGER"); break;
        case LANGUAGE_HANDLEROID: appendStringInfoString(&buf, "LANGUAGE_HANDLER"); break;
        case INTERNALOID: appendStringInfoString(&buf, "INTERNAL"); break;
        case OPAQUEOID: appendStringInfoString(&buf, "OPAQUE"); break;
        case ANYELEMENTOID: appendStringInfoString(&buf, "ANYELEMENT"); break;
        case ANYNONARRAYOID: appendStringInfoString(&buf, "ANYNONARRAY"); break;
        case ANYENUMOID: appendStringInfoString(&buf, "ANYENUM"); break;
        case FDW_HANDLEROID: appendStringInfoString(&buf, "FDW_HANDLER"); break;
        case INDEX_AM_HANDLEROID: appendStringInfoString(&buf, "INDEX_AM_HANDLER"); break;
        case TSM_HANDLEROID: appendStringInfoString(&buf, "TSM_HANDLER"); break;
        case TABLE_AM_HANDLEROID: appendStringInfoString(&buf, "TABLE_AM_HANDLER"); break;
        case ANYRANGEOID: appendStringInfoString(&buf, "ANYRANGE"); break;
        case BOOLARRAYOID: appendStringInfoString(&buf, "BOOLARRAY"); break;
        case BYTEAARRAYOID: appendStringInfoString(&buf, "BYTEAARRAY"); break;
        case CHARARRAYOID: appendStringInfoString(&buf, "CHARARRAY"); break;
        case NAMEARRAYOID: appendStringInfoString(&buf, "NAMEARRAY"); break;
        case INT8ARRAYOID: appendStringInfoString(&buf, "INT8ARRAY"); break;
        case INT2ARRAYOID: appendStringInfoString(&buf, "INT2ARRAY"); break;
        case INT2VECTORARRAYOID: appendStringInfoString(&buf, "INT2VECTORARRAY"); break;
        case INT4ARRAYOID: appendStringInfoString(&buf, "INT4ARRAY"); break;
        case REGPROCARRAYOID: appendStringInfoString(&buf, "REGPROCARRAY"); break;
        case TEXTARRAYOID: appendStringInfoString(&buf, "TEXTARRAY"); break;
        case OIDARRAYOID: appendStringInfoString(&buf, "OIDARRAY"); break;
        case TIDARRAYOID: appendStringInfoString(&buf, "TIDARRAY"); break;
        case XIDARRAYOID: appendStringInfoString(&buf, "XIDARRAY"); break;
        case CIDARRAYOID: appendStringInfoString(&buf, "CIDARRAY"); break;
        case OIDVECTORARRAYOID: appendStringInfoString(&buf, "OIDVECTORARRAY"); break;
        case JSONARRAYOID: appendStringInfoString(&buf, "JSONARRAY"); break;
        case XMLARRAYOID: appendStringInfoString(&buf, "XMLARRAY"); break;
        case POINTARRAYOID: appendStringInfoString(&buf, "POINTARRAY"); break;
        case LSEGARRAYOID: appendStringInfoString(&buf, "LSEGARRAY"); break;
        case PATHARRAYOID: appendStringInfoString(&buf, "PATHARRAY"); break;
        case BOXARRAYOID: appendStringInfoString(&buf, "BOXARRAY"); break;
        case POLYGONARRAYOID: appendStringInfoString(&buf, "POLYGONARRAY"); break;
        case LINEARRAYOID: appendStringInfoString(&buf, "LINEARRAY"); break;
        case FLOAT4ARRAYOID: appendStringInfoString(&buf, "FLOAT4ARRAY"); break;
        case FLOAT8ARRAYOID: appendStringInfoString(&buf, "FLOAT8ARRAY"); break;
        case CIRCLEARRAYOID: appendStringInfoString(&buf, "CIRCLEARRAY"); break;
        case MONEYARRAYOID: appendStringInfoString(&buf, "MONEYARRAY"); break;
        case MACADDRARRAYOID: appendStringInfoString(&buf, "MACADDRARRAY"); break;
        case INETARRAYOID: appendStringInfoString(&buf, "INETARRAY"); break;
        case CIDRARRAYOID: appendStringInfoString(&buf, "CIDRARRAY"); break;
        case MACADDR8ARRAYOID: appendStringInfoString(&buf, "MACADDR8ARRAY"); break;
        case ACLITEMARRAYOID: appendStringInfoString(&buf, "ACLITEMARRAY"); break;
        case BPCHARARRAYOID: appendStringInfoString(&buf, "BPCHARARRAY"); break;
        case VARCHARARRAYOID: appendStringInfoString(&buf, "VARCHARARRAY"); break;
        case DATEARRAYOID: appendStringInfoString(&buf, "DATEARRAY"); break;
        case TIMEARRAYOID: appendStringInfoString(&buf, "TIMEARRAY"); break;
        case TIMESTAMPARRAYOID: appendStringInfoString(&buf, "TIMESTAMPARRAY"); break;
        case TIMESTAMPTZARRAYOID: appendStringInfoString(&buf, "TIMESTAMPTZARRAY"); break;
        case INTERVALARRAYOID: appendStringInfoString(&buf, "INTERVALARRAY"); break;
        case TIMETZARRAYOID: appendStringInfoString(&buf, "TIMETZARRAY"); break;
        case BITARRAYOID: appendStringInfoString(&buf, "BITARRAY"); break;
        case VARBITARRAYOID: appendStringInfoString(&buf, "VARBITARRAY"); break;
        case NUMERICARRAYOID: appendStringInfoString(&buf, "NUMERICARRAY"); break;
        case REFCURSORARRAYOID: appendStringInfoString(&buf, "REFCURSORARRAY"); break;
        case REGPROCEDUREARRAYOID: appendStringInfoString(&buf, "REGPROCEDUREARRAY"); break;
        case REGOPERARRAYOID: appendStringInfoString(&buf, "REGOPERARRAY"); break;
        case REGOPERATORARRAYOID: appendStringInfoString(&buf, "REGOPERATORARRAY"); break;
        case REGCLASSARRAYOID: appendStringInfoString(&buf, "REGCLASSARRAY"); break;
        case REGTYPEARRAYOID: appendStringInfoString(&buf, "REGTYPEARRAY"); break;
        case REGROLEARRAYOID: appendStringInfoString(&buf, "REGROLEARRAY"); break;
        case REGNAMESPACEARRAYOID: appendStringInfoString(&buf, "REGNAMESPACEARRAY"); break;
        case UUIDARRAYOID: appendStringInfoString(&buf, "UUIDARRAY"); break;
        case PG_LSNARRAYOID: appendStringInfoString(&buf, "PG_LSNARRAY"); break;
        case TSVECTORARRAYOID: appendStringInfoString(&buf, "TSVECTORARRAY"); break;
        case GTSVECTORARRAYOID: appendStringInfoString(&buf, "GTSVECTORARRAY"); break;
        case TSQUERYARRAYOID: appendStringInfoString(&buf, "TSQUERYARRAY"); break;
        case REGCONFIGARRAYOID: appendStringInfoString(&buf, "REGCONFIGARRAY"); break;
        case REGDICTIONARYARRAYOID: appendStringInfoString(&buf, "REGDICTIONARYARRAY"); break;
        case JSONBARRAYOID: appendStringInfoString(&buf, "JSONBARRAY"); break;
        case JSONPATHARRAYOID: appendStringInfoString(&buf, "JSONPATHARRAY"); break;
        case TXID_SNAPSHOTARRAYOID: appendStringInfoString(&buf, "TXID_SNAPSHOTARRAY"); break;
        case INT4RANGEARRAYOID: appendStringInfoString(&buf, "INT4RANGEARRAY"); break;
        case NUMRANGEARRAYOID: appendStringInfoString(&buf, "NUMRANGEARRAY"); break;
        case TSRANGEARRAYOID: appendStringInfoString(&buf, "TSRANGEARRAY"); break;
        case TSTZRANGEARRAYOID: appendStringInfoString(&buf, "TSTZRANGEARRAY"); break;
        case DATERANGEARRAYOID: appendStringInfoString(&buf, "DATERANGEARRAY"); break;
        case INT8RANGEARRAYOID: appendStringInfoString(&buf, "INT8RANGEARRAY"); break;
        case CSTRINGARRAYOID: appendStringInfoString(&buf, "CSTRINGARRAY"); break;
        default: appendStringInfo(&buf, "%d", oid);
    }
    return buf.data;
}
