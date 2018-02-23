# A brief intro to Plan 9 with no assumptions about prior knowledge

So, you have successfully booted a Plan 9 system (either a VM or native) and have more or less a blank screen staring back at you. What next? The very first thing to know is how to quit safely. The command

	fshalt

stops the root fileserver and should always be used prior to exiting the system. "fshalt -r" will stop the root fs and then reboot.

## Rio: The graphical user interface

Rio (or "grio" in the case of ANTS) is the main interface/window manager for Plan 9. It does not implement an interface with icons and/or "pull-down" menus. Instead it mostly just allows you to create/move/resize application windows, which run a textual shell named "rc" by default. Holding the right mouse button down brings up the basic menu of rio controls. In general, you use the right button to select an option from that menu, and then use the right button again to apply it. To make a new window, right-click and select "New" from the menu, then move the mouse cursor to the desired position and press and hold the right button, and "sweep out" the size of the new window.

### rc: The textual shell

The shell is the primary interface to the system. "rc" stands for "run commands" and that is the primary job of the shell, to provide a way to run all the rest of the commands in the operating system. Many of the programs that rc runs are shared with the standard unix environment: "man" to view documentation ("man rc" for instance), "ls" to list files in the current directory ("lc" columnizes the list), "cat" to display the contents of a file, "cd" to change directories, and a slew of other common commands will be listed later. rc will also run graphical programs, which will then take over the window they are running in until the exit or are interrupted. The most important of these is "games/catclock" which should be run frequently and is worthy of deep contemplation. To send an interrupt note to a running program, press the delete key.

### Acme: The editor/directory browser/alternate interface

Acme is an interesting program. It is rather an exception to the general Plan 9/unix philosophy that a program should only have one task. Acme is both a text editor, a directory browser, and a whole alternative interface. It has a different model of mouse usage than the rest of the system, and some people find it integrates poorly into the whole. The main thing to know is that the words in the upper title bars of the windows are commands that will be executed if you middle-click on them. So, to make a new panel, you middle click "New". To save a file that you are editing, you click "Put". Inside panels, right clicking acts in a way similar to following links. If a panel contains a directory listing, right clicking on a subdirectory will open a new panel listing it, and right clicking on a file will open that file in a new panel for editing. If you type "win" and middle click, a new panel will open with the rc shell running.

### Mothra: The simple web browser

Mothra is a browser for "the web that we have lost" - it doesn't do javascript or multimedia, just text and simple pictures, and it eliminates most of the formatting used by modern websites. It has a url bar and history of pages visited at the top of the screen, the left button opens links, and the right mouse button opens a menu. The "save hit" and "hit list" options are for saving and viewing a set of bookmarks.

### Page: The document/image viewer

Page will display pdf, ps, or image files. There are many documents about aspects of the os saved in /sys/doc. "Page /sys/doc/9.ps" opens the primary paper about the design of Plan 9, (which is now well over 20 years old, but mostly still applicable). The left button can be used to drag the document within the containing window, and the right mouse button brings up a navigation menu of the pages. The middle button has an additional menu of command such as resizing. Page can also be used to view files such as .jpg or .png images.

### Winwatch: Window selection

The winwatch program shows a list of windows open within the current rio/grio. Right click on the name of a window to "surface" it, right click again to hide it. Middle-clicking allows you to change the displayed name of a window. Because rio can run inside rio, it is often useful to organize your workspace in terms of multiple subrios which you select between using a winwatch running in the top-level rio. (For convenience, I will be referring to grio as "rio" because grio has all of rio's functionality.)

### Stats: Status monitoring

The stats program provides a graphical real-time view of system resource use. It can view many different aspects, but what I find most useful is to start the program with stats -lems which displays the load (how much work the cpu is doing), the ethernet (how much network traffic there is), the memory usage, and the amount of syscalls (requests to kernel functions.) Feel free to view different menus and do different things with the system and see how they affect what stats displays.

## Working in rc

Feeling comfortable working in a text-based command shell is the mark of a seasoned user of unix-related operating systems. There is a core of commands which are shared between original unix, linux, and Plan 9. Before discussing these shared commands, let's look at something Plan 9 specific:

### Namespace viewing and alteration

#### ns

Type "ns" into a shell to view the namespace that shell is running within. The namespace can be thought of as a map of your environment - what's where. The namespace is displayed as a list of the commands that are used to create it.

#### mount

The mount command adds a new resource into the namespace. The basic syntax is "mount SOURCE TARGET"

#### bind

The bind command makes a piece of the namespace visible at a new name as well as the original. Basic syntax is "bind SOURCE TARGET"

#### flags to mount and bind

The mount and bind commands share several flags. The -c flag means that (given correct permissions) new files can be created in the target of the command. The -a or -b flag makes the mount/bind a "union" - the newly added mount or bind is "merged" with the target and does not replace it. The -a or -b flags choose whether the newly added files are "after" or "before" those that were there originally. In other words, if a file named "foo" exists in both old and new, this specifies which version of foo will you see if you do "cat /target/foo"

#### kernel devices

Many of the commands that construct the namespace use \# as the source. This special character in the pathnames indicates that the source is special files provided by the kernel. The drivers for the computer hardware are provided as \# trees, and several special "synthetic" file systems are also created by the kernel. For instance, the kernel creates a special purpose filesystem for viewing and manipulating running processes named 'p' so the 

	bind  '#p' /proc 

in the namespace makes this kernel-based set of files visible at /proc.

#### The /srv device

One of the kernel devices, bound to /srv has the special function of being the place where userspace (non-kernel) fileservers register themselves and provide a "file descriptor" for access. You will see many of the mounts into the namespace come from \#s, the /srv device. 

#### The root filesystem

The most important of all of these is the root disk-based filesystem which provides the majority of the files you interact with - the programs you run, and the data files you use or create. There are several different disk-based fileservers but they all provide similar functionality to the user. They are started at boot, and place a filedescriptor at /srv/boot, and are then mounted right at the beginning of constructing the namespace.

### Standard shell commands

These are the commands that are mostly equivalent between Plan9/unix/linux. There are some differeces (the linux versions usually have quite a few more options) but the basic use is very similar. A standard convention: if you place a leading / in front of a file path, it is an absolute path from the root. Without a leading slash, it is relative to your current working directory.

* man - the most important command for new users. "man foo" prints documentation about foo.
* lookman - "lookman foo" searches the documentation index for which manpages mention foo and lists them
* ls - list the contents of the current directory. You can also supply a path such as ls /foo/bar to view the contents of bar. "lc" prints in columns. 
* cd - change directory. cd / will put you in the lowest level. just "cd" on its own will put you in your home directory. cd /foo/bar changes to the bar subdirectory of foo.
* pwd - print working directory. Shows the location of your current shell.
* cat - short for "concatenate" but mostly used to view files. "cat foo" dumps the contents of foo to standard output.
* cp - copy. the syntax is "cp OLD NEW" to make NEW a copy of OLD. In plan 9, cp only works on single files, not directories.
* dircp - copy a directory. "dircp OLD NEW" puts the contents of OLD into NEW. NEW must be an already existing directory.
* mv - move a file. "mv OLD NEW" moves OLD to NEW. OLD is deleted by this operation. In plan9, directories cannot be mv.
* rm - remove. "rm TARGET" removes target. If TARGET is a directory, you need to use rm -rf TARGET. Use caution.
* echo - print to standard output. "echo foo" just prints foo.
* ps - view processes. "ps -a" will show all processes and useful information about each.
* grep - search. "grep foo bar" searches file bar for all lines containing the string foo.
* sed - stream edit. The most common use of sed is to search and replace. To make NEWFILE a copy of FILE but with all instances of foo replaced with bar, use this syntax:

	sed 's/foo/bar/g' FILE >NEWFILE

* date - print the current time.
* fortune - print a random piece of wisdom or humor.

### Shell redirections and piping

The most famous innovation of unix is its use of "pipes" - this is part of the toolbox philosphy. Because many shell tools work by default on "standard input" and write to "standard output" they can be chained together. For instance:

	cat /sys/man/7/* |grep are|sed 's/e/EE/g'

This uses a shell wildcard to concatenate all files in section 7 of the manual, and pipe them to grep, which searches for all lines containing the word "are", and print them, and sed receives this as input and substitutes EE for all "e"s. In addition to the | pipe character, the > character redirects output to a file, so adding >EEman.txt to the end of the above command would save the output in that file. The < character means "take input from the specified file".

### Environment variables

If you type

	foo=bar

Then type

	echo $foo

The output produced will be "bar" because you have created a variable named foo with bar as the contents. The current variables defined in the shell are stored in a special directory called /env so if you do

	ls /env

you will see what variables are currently defined in the environment.

