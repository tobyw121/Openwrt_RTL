COV_PATH=/opt/coverity/cov-analysis-linux64-8.0.0/bin
COV_BUILD=$(COV_PATH)/cov-build
COV_ANALYZE=$(COV_PATH)/cov-analyze --security --concurrency --enable ATOMICITY --enable MISSING_LOCK --enable DELETE_VOID --checker-option PASS_BY_VALUE:size_threshold:16 --checker-option USE_AFTER_FREE:allow_simple_use:false --enable-constraint-fpp --enable-callgraph-metrics
COV_FORMAT=$(COV_PATH)/cov-format-errors
COV_INTER=~/cov-$(shell basename $(shell pwd))

all:
	rm -rf $(COV_INTER)
	$(COV_BUILD) --dir $(COV_INTER) make $(target)
	$(COV_ANALYZE) --dir $(COV_INTER) --wait-for-license
	$(COV_FORMAT) --dir $(COV_INTER)
