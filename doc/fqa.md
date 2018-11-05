# ANTS FQA

## Contents

- [overview](#overview)
- [livecd](#livecd)
- [installing from source](#sourceinstall)
- [updating](#updating)
- [kernel namespace manipulation](#kernelnamespace)
- [boot process](#bootprocess)
- [admin namespace](#adminnamespace)
- [rerootwin](#rerootwin)
- [other namespace scripts](#nsscripts)
- [hubfs](#hubfs)
- [grio](#grio)
- [grid utilities](#gridutilities)
- [venti and fossil management](#ventifossil)
- [divergences from 9front](#divergences)
- [preinstalled images](#preinstalled)
- [Bell Labs and 9legacy support](#labslegacy)

## Overview
<a name="overview""></a>

ANTS is a set of tools to structure and manage Plan 9 namespaces to make systems and grids more flexible, reliable, and easy to administer. It primarily supports the 9front distribution of Plan 9, although an older version can be installed to the Bell Labs or 9legacy versions also. ANTS is well suited to cloud based hosting and scripts for use with vultr.com are provided.

9gridchan has a set of services online. They are usable from any client which can mount 9p fileservers - ANTS usage is not required. The services include an irc-like hubfs chat, acme-editable wiki, shared writable ramdisk, read-only root filesystem, publicly writable service registry, and a shared plumber to transmit web, image, and document links to connected clients.

## Live/install cd image
<a name="livecd"></a>

#### Download .iso images

- [32-bit 9ants386.iso.gz](//files.9gridchan.org/9ants386.iso.gz)
- [64-bit 9ants64.iso.gz](//files.9gridchan.org/9ants64.iso.gz)

#### Guide

- [brief usage guide](//doc.9gridchan.org/guides/livecd)

## Installing from source
<a name="sourceinstall"></a>

#### Repository links

- [development repo](https://bitbucket.org/mycroftiv/antshill) : latest code, follows 9front hg tip
- [oldstable repo](https://bitbucket.org/mycroftiv/ants9front) : antsv4.2 builds with 9front release 9front-6350
- [archival repo](https://bitbucket.org/mycroftiv/plan9ants) : old repo from 2017 and before, labs/legacy support is here

#### Instructions

- [instructions](//ants.9gridchan.org/INSTALLING) : instructions for building and installing from source

## Updating
<a name="updating"></a>

ANTS tries to stay current with 9front updates. The general flow for updating is: run the 9front sysupdate script, build/install the changed applications, then pull updates from the "antshill" repository and build/install. Exactly what is required depends on what has been changed. The summary provided below covers cases where the changes are extensive to both userspace and the kernel. The standard flow for extensive 9front userspace updates is:

	sysupdate
	cd /sys/src
	mk clean
	mk install
	mk clean

To then update and install latest ANTS:

	cd /sys/src/ants #or wherever your ANTS repo is located if not installed from livecd
	hg pull https://bitbucket.org/mycroftiv/antshill
	hg update
	rm frontmods/include/libc.rebuild.complete
	build 9front
	build fronthost
	9fs 9fat
	cp 9ants /n/9fat	#or 9ants64
	cp tools.tgz /n/9fat

The above assumes that you did a full mk install in /sys/src and that a new kernel version is desired. In the case of smaller updates where you just built/installed individually updated applications and don't need/want a new kernel version, it is generally only necessary to pull and update the ants repo and then "build fronthost".

## Kernel namespace manipulation
<a name="kernelnamespace"></a>

#### Writable /proc/pid/ns

The ANTS kernel is modified to allow modification of process namespace via the 'ns' files contained in /proc. You may only modify the ns file of processes you own. The modifications are performed by writing standard ns operations to the file.

	echo 'mount -c /srv/kremvax /n/kremvax' >/proc/979/ns
	echo 'bind -bc /n/kremvax/lib /lib' >/proc/979/ns

Process 979 will now see a union of kremvax' lib at /lib and writes will go to kremvax. Note that this example assumes the /srv is mountable without authentication. Authentication during mounts is not supported by this mechanism. As with almost everything in plan9, this operation is network transparent - you can import the /proc of a remote machine and edit the namespace of processes on which you have appropriate write permissions.

#### Independent /srv namespaces

The ANTS kernel provides independent /srv namespaces implemented analogously to be behavior of /env. A process can add the RFCSRVG flag to rfork to receive a new clean /srv. All of its children will share this /srv. The new /srv behaves identically. File descriptors attached to the previous /srv remain usable, so the process and its children continue to have access to the root filesystem and services they have already mounted. The rc shell offers support for

	rfork V

to enter a new /srv namespace.

#### Global zrv device

If different process groups are using independent /srv namespaces, it is sometimes useful to provide services which are available globally. The "zrv" device, available at \#z provides this functionality. It behaves exactly as traditional /srv and is unaffected by the new rfork options. It is not bound anywhere in the namespace by default. If bound to a location other than /srv, programs may require modification to make use of it.

## Boot process
<a name="bootprocess"></a>

The ANTS boot process is rewritten to provide additional options for connecting to the root fileserver, and to create an administrative namespace which is independent of the root fs. It makes use of the kernel's paqfs and an optional tools.tgz stored in 9fat to create a ramdisk-rooted environment with a useful set of binaries. This independent environment runs its own cpu listener, conventionally on port 17060, to provide access.

- [plan9rc manpage](http://ants.9gridchan.org/magic/man2html/8/plan9rc)

## Admin namespace
<a name="adminnamespace"></a>

The ANTS boot/admin namespace is independent of the root fileserver, and offers access to the tools compiled into the kernel paqfs and optionally loaded from an additional rootfs.tgz. The kernel paqfs offers the following programs:

        9660srv                 fossil/fossil           tar
        aan                     cwfs64x                 test
        awk                     grep                    tlsclient
        auth/newns              grio                    tlssrv
        auth/secstore           gunzip                  touch
        aux/kbdfs               hubfs                   tr
        aux/listen1             hubshell                unmount
        aux/wpa                 ip/ipconfig             nusb/usbd
        basename                ip/rexexec              nusb/ether
        bind                    ls                      nusb/disk
        cat                     mkdir                   nusb/kb
        cfs                     mntgen                  venti/venti
        chmod                   mount                   wc
        cp                      mv                      xd
        cpu                     ndb/dnsgetip            fstype
        dd                      hjfs                    diskparts
        disk                    ramfs                   hub
        disk/cryptsetup         rc                      rconnect
        disk/edisk              read                    rcpu
        disk/fdisk              rm                      rexport
        disk/prep               rx                      rimport
        dossrv                  sed                     save9cfg
        echo                    sleep                   srvtls
        ed                      srv                     nusbrc
        exportfs                srvfs                   bootrc

The following additional programs are present in the default tools.tgz:

    9fs            dns            memory         seq            vacfs
    Kill           flfmt          mkfs           sha1sum        vga
    acme           format         mouse          slay           vncs
    as             fortune        netstat        stats          vncv
    authdebug      fossilast      ns             stub           webfs
    authsrv        fshalt         p              sync           window
    changeuser     hget           ping           syncindex      wrarena
    con            hoc            ps             syscall        yesterday
    cs             htmlfmt        pwd            telnet         zerotrunc
    date           import         rdarena        telnetd
    diff           keyfs          realemu        traceroute
    dircp          kill           reboot         vac

## Rerootwin
<a name="rerootwin"></a>

The rerootwin script serves a similar function to "chroot" in unix - it moves into a new namespace built from the specified file in /srv. It allows you to change root while using a remote cpu server without losing the input devices from /mnt/term.

- [Rerootwin manpage](//ants.9gridchan.org/magic/man2html/1/rerootwin)

The central mechanism of "saving" the attached input/output devices via srvfs is provided for independent use by the 'savedevs' and 'getdevs' scripts.

## Other namespace manipulation scripts
<a name="nsscripts"></a>

The ANTS kernel modification to allow process namespace alterations via writes to the ns file can be scripted into higher level operations. A set of scripts to compare and rewrite the namespace of different processes is provided. These scripts are rather primitive and may not always function exactly as intended because they operate on simple textual comparison without a detailed understanding of the dependency of later operations on previous ones.

- [Cpns, addns, subns manpage](http://ants.9gridchan.org/magic/man2html/1/cpns)

There are some additional namespace modification utility scripts which do not depend on the modified ANTS kernel. These scripts simply act to make a remote root fileserver usable in the local context, without a full 'rerootwin' operation. Their main purpose is to allow easy access to the binaries of a full system from the boot/admin namespace.

- [Addwrroot et. al manpage](//ants.9gridchan.org/magic/man2html/1/addwrroot)

## Hubfs
<a name="hubfs"></a>

Hubfs is a lightweight fs which uses pipelike files to provide functionality similar to screen/tmux. It can also be viewed as a "pub/sub" server and used for purposes similar to irc or map-reduce data processing. The ANTS boot process starts a hubfs in the boot/admin namespace, and grio makes use by default of another hubfs within the standard namespace.

- [Hubfs manpage](//ants.9gridchan.org/magic/man2html/4/hubfs)
- [Wiki page](http://9p.io/wiki/plan9/hubfs/index.html)

## Grio
<a name="grio"></a>

Grio is a modified rio with hubfs integration, a user-configurable additional menu command, and full color theming options.

- [Grio manpage](//ants.9gridchan.org/magic/man2html/1/grio)

## Grid utilities
<a name="gridutilities"></a>

9gridchan.org provides a set of public 9p services which work together to create a simple and secure collaborative environment. It offers an irc-like chat, acme-editable wiki, a plumber for web and file links, a shared ramfs, a read-only rootfs, and registries for 9gridchan services and services provided by other users. Most of the documentation is located at the wiki.

- [grid wiki](http://wiki.9gridchan.org/1/index.html)

ANTS includes some scripts and utilities for its use.

- [chat script](//ants.9gridchan.org/hubfs/chat)
- [chat manpage](//ants.9gridchan.org/magic/man2html/1/chat)
- [gridstart script](//ants.9gridchan.org/scripts/gridstart)
- [gridstart manpage](//ants.9gridchan.org/magic/man2html/1/gridstart)
- [gridlisten1 source code](//ants.9gridchan.org/patched/gridlisten1.c)
- [gridlisten1 manpage](//ants.9gridchan.org/magic/man2html/8/gridlisten1)

## Venti and Fossil management
<a name="ventifossil"></a>

ANTS can be used with any root filesystem, but the use of fossil in combination with venti makes it easier to work with multiple independent roots and create them on demand. A goal of ANTS is creating highly reliable grids of systems, so tools are included to assist in replicating data between independent venti servers and tracking fossil rootscores.

- [ventiprog manpage](//ants.9gridchan.org/magic/man2html/8/ventiprog)

The ramfossil script creates a ramdisk which servers fossil from the given rootscore. Because fossil only retrieves blocks from venti as they are actually used, even a relatively small ramdisk can provide access to a much larger root filesystem.

- [ramfossil script](//ants.9gridchan.org/scripts/ramfossil)

There are additional fossil utility scripts for managing rootscores and some other operations.

## Divergences from 9front
<a name="divergences"></a>

In most aspects, ANTS adds additional features without changing the standard behavior and configuration of 9front. There are a few administrative/configuration differences which should be mentioned, however. The following lists some of the most notable. They are mostly the result of additional configuration variables in plan9.ini to set up the ants boot/admin namespace.

#### Privpassword and cpu service

If the variable "privpassword=" is set to a password in plan9.ini, this is used to create a factotum key for cpu service, which runs from both the ants boot/admin namespace and (if ANTS was installed from the iso) in the main namespace on the standard cpu port, even if service is set to terminal. If privpassword is not set and no other key is provided, the listeners still run but will not be usable. The 9fat containing plan9.ini and the /dev/kmesg which prints plan9.ini variables are hostowner-access only and must not be publicly exposed if privpassword= is used.

If you convert an install from terminal to cpu service, you probably want to remove the privpassword= setting from plan9.ini and use only the machine key from nvram read by factotum at boot.

#### Static IP configuration

Because of the boot/admin cpu listener, as well as the possible use of venti/fossil, ipconfig in ANTS usually happens in boot prior to mounting the rootfs. As a result, static ip needs to be configured via plan9.ini variables. To use a static ip, add the following variables with correct values to plan9.ini. (The iso installer will do this for you if you select static ip configuration at install time).

* ipaddress=
* ipmask=
* gateway=

#### System name configuration

Also due to the boot/admin namespace and early ipconfig, the system name should be set with a plan9.ini variable. Simply set sysname= in plan9.ini. You still need to match this name in your /lib/ndb/local configuration.

## Preinstalled images
<a name="preinstalled"></a>

ANTS user henesy has contributed a pair of preinstalled images for use with virtual machines. There is a vbox (virtualbox) and qcow2 (qemu) format image of the 64-bit install of the "Gridscape Navigator" release of 31 January 2018.

- [preinstalled vm images](http://9.postnix.us/9ants/)

The images include a preset password for remote access, so the first step on booting them should be to change the password, stored in the plan9.ini file 9fat. "9fs 9fat" and edit plan9.ini to chage the privpassword= to a secure personal password, save, and then fshalt -r to reboot.

## Bell Labs and 9legacy support
<a name="labslegacy"></a>

The current ANTS repos do not include support for Bell Labs or 9legacy versions of the operating system. Support was dropped at the end of 2017. The old repo, which should still build and be usable with Bell Labs, still exists.

- [old plan9ants repo](https://bitbucket.org/mycroftiv/plan9ants)

