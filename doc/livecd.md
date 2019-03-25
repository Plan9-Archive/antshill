# ANTS/9front Live+Install CD .iso image

### ANTSv5.23 386 and 64-bit iso based on 9front r7098, released 25 Mar 2019

## [files.9gridchan.org/9ants386.iso.gz](//files.9gridchan.org/9ants386.iso.gz)

uncompressed md5sum: 55359245a7de1cc46140c9392f4958f1

## [files.9gridchan.org/9ants64.iso.gz](//files.9gridchan.org/9ants64.iso.gz)

uncompressed md5sum: 76155f4057cd8a518e02a90e7a483e96

The Advanced Namespace Tools for Plan 9 are now available for testing and installation as a custom spin of the 9front live/install cd image. The cd boots the 9ants custom kernel and includes all userspace tools, and can install the full ANTS system. Installation is the same as standard 9front, the command inst/start beings the process. The installer also has new optional abilities to setup a cpu/auth server rather than a terminal, and also provides the option for venti+fossil in addition to standard 9front fileservers. You can experiment with most of the new features without needing to install.

### New features and applications

#### Grid tools

* A script to connect to the 9gridchan.org public 9p services.

Just type

	gridstart

to mount public resources and start a subrio within that namespace. Add the -t flag to use tls for the connections. The -m flag will dial and mount the services without starting a subrio or launching applications.

* A chat script that provides irc-like service in collaboration with hubfs.

The chat script requires access to a /srv/chat file provided by a hubfs containing a 'chat' file. The public 9gridchan.org services include a chat service, started as part of gridstart. To use it on its own:

	srv tcp!chat.9gridchan.org!9997 chat
	chat

* A gridlisten1 program which functions analogously to aux/listen1 but which also announces services to an inferno-hosted registry. A world-writable registry (different from the base 9gridchan registry) is part of the services. To perform a read-only share of a directory and announce it:

	srv -c tcp!registry.9gridchan.org!7675 pubreg /mnt/registry
	myip=public.ip.or.domain
	gridlisten1 -t tcp!\*!9898 /bin/exportfs -R -r /tmp/publicshare

#### Kernel

* The namespace of processes can be manipulated via the /proc/pid/ns file.

To change the namespace of a process you own

	echo 'bind /foo /bar' >/proc/pid/ns

where "pid" is the process id. Binds, mounts, unmounts and the standard flags are supported. Mounts requiring a new authentication are not supported.

* Private /srv namespaces are available.

The #s kernel device is handled in the manner of /env. In rc,

	rfork V

will move you into a new, clean /srv. Your existing file descriptors are unaffected, so your existing mounts (such as that of the root fileserver) continue to function. New child processes will inherit this /srv. A global srv which is not forked is available to all processes at #z. The #z device is not bound anywhere in the standard namespace but works analogously to original /srv once it is.

#### Applications: hubfs i/o muxing persistence and grio customized rio

* Hubfs allows persistent shared textual applications similar to tmux/screen
* Can also be used as a general purpose multiclient message queue via 9p

Hubfs provides persistent pipelike buffered files as a 9p filesystem. It is usually used to provide shared access to instances of the rc shell and applications like ircrc. To create a new instance or attach to an existing one, a wrapper script is used like this:

	hub FSNAME

Multiple clients can attach to the same fs, each able to write to it, with reads being sent to all of them. For details on usage, see man hubfs.

* Grio customized rio integrates with hubfs and offers color selection and a customizable command in the menu

The standard rio is launched by the default profile, but the "grio" command will create subrios using the customized ANTS grio. It offers several new features. It adds a "hub" command to the menu, which connects to whatever instance of hubfs is mounted at /n/hubfs. If none is mounted, it creates a new one. It adds the ability to add a custom command of your choice to the menu, by default /bin/acme. -x command -a flags/argument sets the custom command. The argument of -a cannot include spaces. Customizable colors are available and specified via their hex values. Check man grio for full information. Sample command to start a light blue grio with stats -lems in the menu:

	grio-c 0x49ddddff -x /bin/stats -a -lems

#### Independent boot/admin namespace and namespace manipulation scripts

* ANTS boot process creates a separate namespace with no connection to the root fileserver

The modified ANTS boot sequence creates a self-sufficient admin environment using a ramdisk and the kernel's compiled-in paqfs. A full ANTS install to disk also includes additional utilities loaded into the ramdisk from a tools.tgz file stored in the 9fat partition. On the livecd, you can access this namespace either via the hubfs at /srv/hubfs started during boot, or by providing a password to the rcpu listeners on ports 17019 (standard namespace) or 17060 (rootless admin namespace) also started during boot. A standard key-adding command like:

	echo 'key proto=dp9ik user=glenda dom=antslive !password=whatever' >/mnt/factotum/ctl

enables remote access.

* Namespace manipulation scripts such as rerootwin

Several scripts are provided to assist in working with multiple namespaces. The most important is rerootwin, which is somewhat analogous to unix 'chroot' and maintains connection to the input and draw devices. If you drawterm/cpu into the independent boot/admin namespace, you will probably wish to shift into the standard namespace with all applications and services available. To do this, use the following sequence within the admin namespace:

	rerootwin -f boot
	service=con
	. $home/lib/profile
	grio #optional

You now have an environment that behaves the same as the main environment, although if you check your ns, you will see it is constructed rather differently. Other namespace manipulation scripts include "cpns SRC DEST", which uses the writable ns in /proc to attempt to modify the namespace of process DEST to match that of process SRC. This is done with a rather crude textual comparison of the contents of the ns files, so results in may practice may be somewhat unpredictable. See man rerootwin and man cpns for more information.

### Installation

* Install process is the same as for standard 9front with addition of fossil+venti option and cpu/auth configuration option

ANTS is compatible with the standard 9front fileservers, but restores the option of installing fossil and venti because fossil rootscores offer a powerful mechanism for efficiently working with multiple root filesystems. Fossil is generally considered less reliable than the other fileservers, so if you do choose to use Fossil, make sure to have a good backup system for your data. ANTS includes some tools for assisting with replicating data between Venti servers and managing rootscore archives. See man ventiprog for usage example.

* Install process adds a password to plan9.ini to setup remote access

The only additional step in the installer, pwsetup, either adds a value to plan9.ini to provide a password for access to the independent admin namespace. This option does not set up a full authsrv/keyfs system, it just adds the password to factotum for hostowner access on port 17060 or the standard namespace on port 17019. There is also an option to configure the system as a full cpu/auth server which means no gui/rio by default. IMPORTANT: make sure to enter "glenda" at the "authid:" prompt if you are configuring as an authserver, or it botches the whole install. The cpu option also means that the console shell is running in the boot/admin namespace. You can start rio with the command

	gui

at which point you will still be in the namespace outside the root fileserver. You can use the rerootwin -f boot, service=con, . $home/lib/profile, grio sequence to start a subrio that will be rooted conventionally.

* Install makes plan9rc the default booting command/method

The live cd uses an ANTS-customized bootrc, but the full install sets the ANTS plan9rc boot script This script is backward compatible with the standard boot process, but offers several new options and possibilities, including the ability to hook custom commands during the boot process, root to cpu servers with aan for reliability, or even connect to a remote hubfs to control the boot process and announce services to an inferno registry. See man plan9rc for some details, although not all possibilities are currently documented.

### Software additions

Also included are tools for playing interactive fiction text adventures, and a small library of games. Just start the

	fiction

script to use a numeric menu to launch the pre-installed text games. You can examine the /rc/bin/fiction script to see how they are started. There are a few more IF interpreter ports and curses versions of some of them located in repos at bitbucket.org/mycroftiv. Many many more games are available, check [http://ifdb.tads.org](http://ifdb.tads.org) and [http://www.ifarchive.org](http://www.ifarchive.org) as well as competition sites like the yearly ifcomp and spring thing to find more possibilities.

Also installed is the shareware wad file for doom, so 

	games/doom

will launch the standard Doom 1 game without needing to download, place, and rename any .wad files. The livecd also includes a chess program by Umbraticus with some engine ports by QWX. Read "man chess" for usage instructions. Byouki-onna contributed "juku", timed flashcard repetition for learning. Another addition is spew's "aplay" and "volume" music tools located at games/aplay and games/volume. Also included is Kvik's "clone" utility for fcp-style fast copies of full directories. Burnzez tools "walk" and "ci" are included and his replica-scripting "rep" tools as well.

### Who is this for?

ANTS should be regarded as experimental software intended for experienced Plan 9 users. It should behave for the user identically to standard 9front but newer users are recommended to use the standard 9front distribution found at [http://9front.org](http://9front.org). The kernel modifications and nonstandard boot process/environment work well in practice, but more testing is required to guarantee that they do not introduce any instabiity and/or security risks. ANTS is designed to assist in using Plan 9 as a true distributed system with multiple nodes. It works fine on a single, local box, but offers the most benefits when used with multiple systems in accordance with the original Plan 9 distributed design.

#### Credits and Thanks

The vast majority of code on the live/install cd is the same as standard 9front, which builds on the earlier work of Bell Labs. The intention of this release is to offer an easy to test and install ANTS environment for 9front developers and users who are curious about these namespace tools. It is not intended as a new fork or competitor to standard 9front. Thanks especially to Cinap Lenrek for his leadership of the 9front project and generous time and assistance with everything I have needed to learn during the course of ANTS development.

#### Known issues

* If the user enters an incorrect value (non-glenda) for the authid: during the auth/wrkey section of the auth server install option, it will cause the whole install process to fail during the bootsetup step.

* Updating and rebuilding the system using the 9front sysupdate command may result in the loss of some ANTS features, and require rebuilding/reinstalling some of the ANTS toolkit, because ANTS attempts to mostly contain its modifications and not overwrite the standard distribution, so for instance the customized rc with rfork V available will be overwritten if the system is rebuilt with a standard mk install in /sys/src. 

* Some documentation is out of date and documentation is spread out between manpages and multiple places on the website.
