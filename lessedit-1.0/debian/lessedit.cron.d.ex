#
# Regular cron jobs for the lessedit package
#
0 4	* * *	root	[ -x /usr/bin/lessedit_maintenance ] && /usr/bin/lessedit_maintenance
