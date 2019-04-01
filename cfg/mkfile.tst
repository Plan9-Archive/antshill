</$objtype/mkfile
BIN=/$objtype/bin

#rfork ne
#. /$objtype/mkfile
#builddir=`{pwd}

distromod:V:
	cp frontmods/libauthsrv/readnvram.c /sys/src/libauthsrv/readnvram.c
	cp frontmods/rcbin/9fs /rc/bin/9fs
	cp frontmods/rcbin/fshalt /rc/bin/fshalt
	cp frontmods/rcbin/termrc /rc/bin/termrc
	cp frontmods/syslibsysconfigproto/distproto /sys/lib/sysconfig/proto/distproto

9frontcd:V:
	builddir=`{pwd}
	if(~ $objtype amd64){
		if(! test -f /$objtype/9ants64 || ! test -f /$objtype/tools.tgz){
			echo please build 9ants kernel and tools.tgz and place in /$objtype
			exit no.files
		}
	}
	if(~ $objtype 386){
		if(! test -f /$objtype/9ants || ! test -f /$objtype/tools.tgz){
			echo please build 9ants kernel and tools.tgz and place in /$objtype
			exit no.files
		}
	}
	mkdir /sys/src/ants
	mkdir /rc/bin/instants
	bind /root /n/src9
	bind $builddir /n/src9/sys/src/ants
	bind frontmods/instants /n/src9/rc/bin/instants
	bind -b frontmods/syslibdist /n/src9/sys/lib/dist
	bind -b frontmods/usrglendalib /n/src9/sys/lib/dist/usr/glenda/lib
	bind -b frontmods/usrglendabinrc /n/src9/sys/lib/dist/usr/glenda/bin/rc
	bind -ac /dist/plan9front /n/src9
	cd /sys/lib/dist
	mk /tmp/9front.$objtype.iso
	cd $builddir

preprepo:V:
	builddir=`{pwd}
	if(! test -d compiletemp){
		echo 'mkdir compiletemp'	
		mkdir compiletemp
	}
	if(! test -d bootdir.extras/root/n){
		echo 'creating bootdir.extras/root skeleton'
		cd bootdir.extras
		tar xf skel.tar
		cd $builddir
	}
	if(! test -e root/acme/bin/$objtype/win){
		echo 'copying acme binaries'
		cp /acme/bin/$objtype/mkwnew root/acme/bin/$objtype
		cp /acme/bin/$objtype/spout root/acme/bin/$objtype
		cp /acme/bin/$objtype/win root/acme/bin/$objtype
	}
	if(! test -e root/lib/font/bit/lucidasans/NOTICE){
		echo 'copying fonts'
		dircp /lib/font/bit/lucidasans root/lib/font/bit/lucidasans
		dircp /lib/font/bit/lucm root/lib/font/bit/lucm
		cp /lib/font/bit/lucm/latin1.9 bootdir.extras/root/lib/font/bit/lucm
		cp /lib/font/bit/lucm/latin1.9.font bootdir.extras/root/lib/font/bit/lucm
		cp /lib/font/bit/lucm/unicode.9.font bootdir.extras/root/lib/font/bit/lucm
	}
	if(! test -e doc/ants.ps)
		troff -ms doc/antspaper.ms | dpost >doc/ants.ps
	rm -rf compiletemp/*

clean:V:
	builddir=`{pwd}
	echo building mk clean
	cd grio
	mk clean
	cd $builddir
	cd hubfs
	mk clean
	cd $builddir
	rm root/bin/* >/dev/null >[2=1]
	rm -rf compiletemp/*

preplibs:V:
	builddir=`{pwd}
	if(! test -e /sys/include/libc.h.orig){
		echo 'backing up libc and lib9p and /lib/namespace'
		cp /sys/include/libc.h /sys/include/libc.h.orig
		cp /sys/src/lib9p/srv.c /sys/src/lib9p/srv.c.orig
		cp /lib/namespace /lib/namespace.orig
	}
	if(! test -e /$objtype/bin/rc.orig){
		echo 'backing up rc at rc.orig'
		cp /$objtype/bin/rc /$objtype/bin/rc.orig
	}
	if(! test -e frontmods/include/libc.rebuild.complete){
		echo 'rebuilding libs with modified libc and lib9p'
		sleep 5
		cp frontmods/lib/namespace /lib/namespace
		cp frontmods/include/libc.h /sys/include/libc.h
		cp frontmods/lib9p/srv.c /sys/src/lib9p/srv.c
		cd /sys/src
		mk cleanlibs
		mk libs
		mk cleanlibs
		cd $builddir
		touch frontmods/include/libc.rebuild.complete
	}
	echo preplibs made

frontpatched:V:
	builddir=`{pwd}
	echo building patched
	ramfs
	mkdir /tmp/rc
	dircp /sys/src/cmd/rc /tmp/rc
	cp frontmods/rc/plan9.c /tmp/rc/plan9.c
	cd /tmp/rc
	mk install
	cd $builddir
	mkdir /tmp/factotum
	dircp /sys/src/cmd/auth/factotum /tmp/factotum
	cp frontmods/factotum/* /tmp/factotum
	cd /tmp/factotum
	mk $O.factotum
	cp $O.factotum $builddir/bootdir.extras/factotum
	cd $builddir
	strip bootdir.extras/factotum >/dev/null >[2=1]
	rm -rf /tmp/factotum

extras:V:
	builddir=`{pwd}
	echo building extras
	cd grio
	mk clean
	mk
	cp $O.out ../bootdir.extras/grio
	cd ../hubfs
	mk clean
	mk all
	cp $O.hubfs ../bootdir.extras/hubfs
	cp $O.hubshell ../bootdir.extras/hubshell
	cp hub ../bootdir.extras
	cd $builddir
	strip bootdir.extras/grio >/dev/null >[2=1]
	strip bootdir.extras/hubfs >/dev/null >[2=1]
	strip bootdir.extras/hubshell >/dev/null >[2=1]

checkfrontclean:V:
	if(~ $objtype 386){
		if(test -e /sys/src/9/pc/9pc){
			echo 'please mk clean in /sys/src/9/pc before building'
			exit
		}
	}
	if(~ $objtype amd64){
		if(test -e /sys/src/9/pc64/9pc64){
			echo 'please mk clean in /sys/src/9/pc64 before building'
			exit
		}
	}
	echo frontclean checked

9front:V: checkfrontclean extras frontpatched tools
	builddir=`{pwd}
#	checkfrontclean
#	cd $builddir
#	build extras
#	build frontpatched
#	build tools
	cd bootdir.extras
	rm skel.tar
	tar cf skel.tar root
	cd $builddir
	bind -bc frontmods/boot /sys/src/9/boot
	bind -bc frontmods/port /sys/src/9/port
	bind -bc frontmods/pc /sys/src/9/pc
	bind -bc frontmods/pc64 /sys/src/9/pc64
	bind -bc compiletemp /sys/src/9/pc
	if(~ $objtype amd64){
		unmount compiletemp /sys/src/9/pc
		bind -bc compiletemp /sys/src/9/pc64
	}
	bind . /n/rootless
	cd /sys/src/9/pc
	if(~ $objtype amd64)
		cd /sys/src/9/pc64
	mk
	cd $builddir
	if(test -e compiletemp/9pc){
		cp compiletemp/9pc 9ants
		echo 9ants kernel built
	}
	if(test -e compiletemp/9pc64){
		cp compiletemp/9pc64 9ants64
		echo 9ants64 kernel built
	}

frontkernel:V:
	builddir=`{pwd}
#	checkfrontclean
	cd $builddir
	build extras
	build frontpatched
	cd bootdir.extras
	rm skel.tar
	tar cf skel.tar root
	cd $builddir
	bind -bc frontmods/boot /sys/src/9/boot
	bind -bc frontmods/port /sys/src/9/port
	bind -bc frontmods/pc /sys/src/9/pc
	bind -bc frontmods/pc64 /sys/src/9/pc64
	bind -bc compiletemp /sys/src/9/pc
	if(~ $objtype amd64){
		unmount compiletemp /sys/src/9/pc
		bind -bc compiletemp /sys/src/9/pc64
	}
	bind . /n/rootless
	cd /sys/src/9/pc
	if(~ $objtype amd64)
		cd /sys/src/9/pc64
	mk
	cd $builddir
	if(test -e compiletemp/9pc){
		cp compiletemp/9pc 9ants
		echo 9ants kernel built
	}
	if(test -e compiletemp/9pc64){
		cp compiletemp/9pc64 9ants64
		echo 9ants64 kernel built
	}

fronthost:V: extras
	builddir=`{pwd}
	echo 'installing ANTS userspace tools'
	cd hubfs
	mk install
	cd ..
	cd patched
	$CC -FTVw gridlisten.c
	$LD -o gridlisten gridlisten.$O
	cp gridlisten /$objtype/bin
	$CC -FTVw gridlisten1.c
	$LD -o gridlisten1 gridlisten1.$O
	cp gridlisten1 /$objtype/bin
	cd ..
	cp bootdir.extras/grio /$objtype/bin
	cp scripts/* /rc/bin
	cp bootdir.extras/namespace.saveterm /lib/namespace.saveterm
	cp bootdir.extras/namespace.save9front /lib/namespace.save9front
	cp /sys/man/1/rc /sys/man/1/rc.orig
	cp /sys/man/2/fork /sys/man/2/fork.orig
	cp /sys/man/3/srv /sys/man/3/srv.orig
	cp /sys/man/3/proc /sys/man/3/proc.orig
	cp sys/man/1/grio /sys/man/1/grio
	cp sys/man/1/cpns /sys/man/1/cpns
	cp sys/man/1/rerootwin /sys/man/1/rerootwin
	cp sys/man/1/addwrroot /sys/man/1/addwrroot
	cp sys/man/1/rc /sys/man/1/rc
	cp sys/man/1/chat /sys/man/1/chat
	cp sys/man/1/gridstart /sys/man/1/gridstart
	cp sys/man/2/fork /sys/man/2/fork
	cp sys/man/3/srv /sys/man/3/srv
	cp sys/man/3/proc /sys/man/3/proc
	cp sys/man/8/plan9rc /sys/man/8/plan9rc
	cp sys/man/8/ventiprog /sys/man/8/ventiprog
	cp sys/man/8/gridlisten1 /sys/man/8/gridlisten1
	cp doc/antspaper.ms /sys/doc/ants.ms
	cp doc/ants.ps /sys/doc/ants.ps
	echo 'ANTS userspace installed, if using ANTS kernel add this to the end of /rc/bin/termrc:'
	echo 'home=/usr/$user; cd; . $home/lib/profile'

tools:VE:
	builddir=`{pwd}
	echo building tools
	rm root/bin/* >/dev/null >[2=1]
	cp cfg/toolcopy root/bin/_toolcopy
	cd root/bin
	. _toolcopy
	cd $builddir
	strip root/bin/* >/dev/null >[2=1]
	ramfs -m /tmp
	tar cf /tmp/tools.tar root
	gzip -9 -c /tmp/tools.tar >tools.tgz
	rm /tmp/tools.tar
	cd $builddir
	echo tools built
