#
# Regular cron jobs for the timelapse package
#
0 4	* * *	root	[ -x /usr/bin/timelapse_maintenance ] && /usr/bin/timelapse_maintenance
