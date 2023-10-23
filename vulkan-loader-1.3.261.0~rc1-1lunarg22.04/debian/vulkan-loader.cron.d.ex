#
# Regular cron jobs for the vulkan-loader package
#
0 4	* * *	root	[ -x /usr/bin/vulkan-loader_maintenance ] && /usr/bin/vulkan-loader_maintenance
