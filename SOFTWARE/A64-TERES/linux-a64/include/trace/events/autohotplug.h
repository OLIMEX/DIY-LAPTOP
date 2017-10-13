#undef TRACE_SYSTEM
#define TRACE_SYSTEM autohotplug

#if !defined(_TRACE_AUTOHOTPLUG_H) || defined(TRACE_HEADER_MULTI_READ)
#define _TRACE_AUTOHOTPLUG_H
#include <linux/tracepoint.h>

/*
 * Tracepoint for cpu_autohotplug.
 */
TRACE_EVENT(autohotplug_operate,

	TP_PROTO(int cpu, int up),
	TP_ARGS(cpu, up),

	TP_STRUCT__entry(
		__field(int, cpu)
		__field(int, up)
	),

	TP_fast_assign(
		__entry->cpu = cpu;
		__entry->up  = up;
	),

	TP_printk("cpu=%d up=%d", __entry->cpu, __entry->up)
);

#endif /* _TRACE_AUTOHOTPLUG_H */

/* This part must be outside protection */
#include <trace/define_trace.h>
