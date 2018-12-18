# Spawngrid: on-demand Plan 9 environments

### 2 December 2018

The spawning grid is a worldwide network of Plan 9 servers which provide on-demand cpu environments from a library of stored filesystem snapshots. Users can spawn and cpu in to container-like independent namespaces with a private root filesystem. Multiple environments can be spawned on the same or different grid servers. New fs snapshots can be created at any time, and also shared with other users on the grid.

The spawning grid is currently invite-only to existing Plan 9 community members - if you use Plan 9 and participate in the 9gridchan public grid, or Plan 9 related freenode irc channels, mailing lists, discord, whatever - you are invited to email mycroftiv at sphericalharmony.com with a username and password request. Doing so indicates that you understand these free services are for legal, non-commercial personal use and you won't use them for anything uncool. No guarantees are made about availability or data retention and service may be terminated at will, but I'm hoping that this service and all data created will be available for many years.

## Using the spawngrid

After an account has been created, add the spawngrid server information to your /lib/ndb/local. It can be found at [files.9gridchan.org/gridndb](//files.9gridchan.org/gridndb), or after the end of the text of this page. Then download the gridorc script from [ants.9gridchan.org/grid/gridorc](//ants.9gridchan.org/grid/gridorc) or copy it from the bottom of this page. Run gridorc as follows:

	gridorc -u gridusername -h gridstorageserver

To choose a value for 'gridstorageserver' check the grid ndb info. There are servers in Frankfort (Germany) Paris (France) New Jersey (USA) Chicago (USA) and San Francisco (USA). The storage servers all begin with a location identifier followed by 'spasto' - so if you want the servers in Paris, the -h parameter for gridorc should be 'parspasto'. gridorc is an interactive program to control a grid storage server and its attached cpu. The commands are as follows:

	scores

Prints a list of filesystem roots available for your user to spawn.

	spawn FSNAME	# or spawndisk FSNAME

The spawn or spawndisk command starts a fileserver for the named rootscore and then begins serving a namespace rooted within it from the cpu server. The spawn command creates a small (100mb) ramdisk to cache fs data, the spawndisk command uses a somewhat larger (1gb) disk partition for temporary data storage. After a successful spawn, the rcpu command to access the spawned environment will be printed. Execute that command in a separate window to use your system.

	save FSNAME NEWNAME	# or savedisk FSNAME NEWNAME

This command saves a new snapshot of your filesystem. Unless the 'save' command is issued, changes to your fs will NOT be saved, and the save command saves only the changes written to the fs at the time it is issued. If the environment was spawned with 'spawndisk' it must be saved with 'savedisk' not just 'save'. Once an fs snapshot is saved and named, it is immutable - you will always receive the same state for spawning that name. To preserve a change in state, the appropriate version of the save command must be used.

	invite USERNAME FSNAME

This command adds a user to fs, places them in the sys group, saves a snapshot of the fs, and adds that scorename to that users' available rootscores. Note that the invite-saved snapshot won't be available to the user who created it - an additional 'save' command would need to be issued. Also, the invite command must be used with fses created with 'spawn' and not with 'spawndisk' at the moment.

	boom FSNAME

This command frees the resources associated with an environment. Users are free to allow their environments to persist without being 'boom'ed, but if the server capacity becomes filled, long-running environments might be terminated to make room for new spawns. Users who are creating multiple environments are encouraged to 'boom' them if they are no longer serving an active purpose. The 'boom' command frees the resources of an active environment, it does not delete the associated snapshot, which can always be re-spawned.

	status

The status command lists the currently active environments being provided by the storage+cpu pair. The command

	exit

terminates the gridorc. Spawned environments will continue exist and be available until and unless they are boomed.

### ndb information

	authdom=spawngrid auth=isoauthtest
	sys=isoauthtest ip=207.148.13.142
	sys=fraspasto ip=199.247.16.96
	sys=fraspacpu ip=45.77.53.117
	sys=parspasto ip=95.179.211.134
	sys=parspacpu ip=95.179.211.111
	sys=njspasto ip=45.76.9.59
	sys=njspacpu ip=207.246.82.44
	sys=chispasto ip=207.148.8.51
	sys=chispacpu ip=66.42.112.142
	sys=sfspasto ip=144.202.97.135
	sys=sfspacpu ip=45.77.6.77

### gridorc script

	#!/bin/rc
	# user facing grid orchestration helper
	
	rfork
	griduser=$user
	while (~ $1 -*){
		switch($1){
		case -u
			griduser=$2
			shift
		case -d
			reqdial=$2
			shift
		case -h
			targhost=$2
			shift
		}
		shift
	}
	if(~ $targhost tcp!*)
		hostdial=$targhost
	if not
		hostdial=tcp!^$targhost^!16998
	if(~ $#reqdial 0)
		reqdial=tcp!^$targhost^!16999
	if(! test -e /n/g/sto.in)
		srvtls $hostdial $targhost.gridsvc.$pid /n/g
	if(! test -e /n/g/sto.in){
		echo no grid service found at /n/g >[2=1]
		exit
	}
	echo 'scores | spawn scorename | save oldname newname | invite user scorename | boom scorename | status'
	cat /n/g/sto.out &
	catkill1=$apid
	cat /n/g/sto.err &
	catkill2=$apid
	cat /n/g/cpu.out &
	catkill3=$apid
	cat /n/g/cpu.err &
	catkill4=$apid
	
	fn killcats{
		@{echo kill>/proc/$catkill1/ctl}
		@{echo kill>/proc/$catkill2/ctl}
		@{echo kill>/proc/$catkill3/ctl}
		@{echo kill>/proc/$catkill4/ctl}
	}
	
	while(usercmd=`{read}){
		switch($usercmd){
		case exit
			killcats
			exit
		case *
			tlsclient -a $reqdial /bin/echo $usercmd
			sleep 1
			echo $griduser req >>/n/g/sto.in
		}
	}
