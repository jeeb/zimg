#
# Regular cron jobs for the zimg package
#
0 4	* * *	root	[ -x /usr/bin/zimg_maintenance ] && /usr/bin/zimg_maintenance
