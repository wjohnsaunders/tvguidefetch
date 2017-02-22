# Welcome to TvGuideFetch

This is an XMLTV grabber for the Australian Community-driven TV Guide based at www.oztivo.net.

A non-standard mechanism is used to fetch guide data for individual channels and days, and then compose a full XMLTV file from each of the fetches. This mechanism results in a massive bandwidth saving as most files remain unchanged after the initial fetch. The program makes use of the "last modified" attribute in the datalist file so that repeated runs of the program typically result in no network activity to check for file changes. This grabber has been written to make maximum use of this facility to not only reduce load on the www.oztivo.net servers, but to make fetching guide data very fast.

This software makes use of libexpat, libcurl and zlib. Please ensure that your system has the development files for these 3 libraries installed before trying to build this software. The Windows builds are created using MinGW with these libraries installed.

Note to MythTV users. When a source is configured to use the grabber, MythTV will run the grabber with the --configure option. New versions of MythTV display grabber output in a window, however older versions require that you alt-tab to the shell from which you ran mythtv-setup. The grabber will prompt for which default config to use, a cache directory location, and your oztivo login details. Default configs are provided for Freeview (Sydney), Analogue (again Sydney) and Foxtel. The suggested cache location is /home/<login>/.xmltv/tvguide_cache.

Note to MythTV users. It is a little known secret that the channel number can be imported into MythTV by specifying a second <display-name> node inside the <channel> node. IceTV uses a <lcn> node for this purpose, which is useless for MythTV. So when you specify the channel number in a <lcn> node this grabber will translate that to a second <display-name> node and hey presto mythfilldatabase imports all your channels with the proper channel number. This makes creating Foxtel channels a breeze. See the sample-configs directory.
