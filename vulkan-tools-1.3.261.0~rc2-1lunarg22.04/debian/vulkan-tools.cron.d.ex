#
# Regular cron jobs for the vulkan-tools package
#
0 4	* * *	root	[ -x /usr/bin/vulkan-tools_maintenance ] && /usr/bin/vulkan-tools_maintenance
