MODULE_big = pg_task
OBJS = conf.o tick.o task.o
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
