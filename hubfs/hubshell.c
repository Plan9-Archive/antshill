#include <u.h>
#include <libc.h>
#include <ctype.h>

/* hubshell is the client for hubfs, usually started by the hub wrapper script */
/* it handles attaching and detaching from hub-connected rcs and creating new ones */

enum smallbuffer{
	SMBUF = 512,
};

enum{
	Detach,
	Remote,
	Local,
	Attach,
	Err,
	In,
	Out,
	Fortun,
	Unfort,
	Echoes,
	Unecho,
	Eof,
	Freeze,
	Melt,
	Fear,
	Calm,
	Trunc,
	Notrunc,
	List,
	Status,
	NCmd,
};

char *cmdstr[] = {
	[Detach] = "detach",
	[Remote] = "remote",
	[Local] = "local",
	[Attach] = "attach",
	[Err] = "err",
	[In] = "in",
	[Out] = "out",
	[Fortun] = "fortun",
	[Unfort] = "unfort",
	[Echoes] = "echoes",
	[Unecho] = "unecho",
	[Eof] = "eof",
	[Freeze] = "freeze",
	[Melt] = "melt",
	[Fear] = "fear",
	[Calm] = "calm",
	[Trunc] = "trunc",
	[Notrunc] = "notrunc",
	[List] = "list",
	[Status] = "status",
	[NCmd] = nil,
};


int
getcmd(char *s)
{
	int i;

	for(i=0; i<NCmd; i++){
		if(strncmp(cmdstr[i], s, strlen(cmdstr[i])) == 0)
			break;
	}

	return i;
}

typedef struct Shell Shell;

/* A Shell opens fds of 3 hubfiles and bucket-brigades data between the hubfs and std i/o fds*/
struct Shell {
	int fd[3];
	long fddelay[3];
	char basename[SMBUF];
	char *fdname[3];
	char shellctl;
	char cmdresult;
};

int notereceived;

/* string storage for names of hubs and paths */
char initname[SMBUF];
char hubdir[SMBUF];
char srvname[SMBUF];
char ctlname[SMBUF];
char basehub[SMBUF];

/* flags for rc commands to flush buffers */
int fortunate;
int echoes;

Shell* setupshell(char*);
void startshell(Shell*);
void fdread(int, Shell*);
void fdinput(int, Shell*);
void closefds(Shell*);
void parsebuf(Shell*, char*, int);

void*
emalloc(ulong sz)
{
	void *v;

	if((v = malloc(sz)) == nil)
		sysfatal("emalloc: %r");
	memset(v, 0, sz);

	setmalloctag(v, getcallerpc(&sz));
	return v;
}

/* set shellgroup variables and open file descriptors */
Shell*
setupshell(char *name)
{
	Shell *s;
	int i;

	s = emalloc(sizeof(*s));
	strncat(s->basename, name, SMBUF);
	for(i = 1; i < 3; i++){
		if((s->fdname[i] = smprint("%s%d", s->basename, i)) == nil)
			sysfatal("smprint: %r");
		if((s->fd[i] = open(s->fdname[i], OREAD)) < 0){
			fprint(2, "hubshell: giving up on task - cant open %s\n", s->fdname[i]);
			return nil;
		};
	}
	s->fddelay[2] = 30;
	if((s->fdname[0] = smprint("%s%d", s->basename, 0)) == nil)
		sysfatal("smprint: %r");
	sprint(s->fdname[0], "%s%d", s->basename, 0);
	if((s->fd[0] = open(s->fdname[0], OWRITE)) < 0){
		fprint(2, "hubshell: giving up on task - cant open %s\n", s->fdname[0]);
		return nil;
	}
	sprint(basehub, s->fdname[0] + strlen(hubdir));
	s->cmdresult = 'a';
	return s;
}

/* fork two reader procs for fd1 and 2, start writing to fd0 */
void
startshell(Shell *s)
{
	/* fork cats for each file descriptor used by a shell */
	if(rfork(RFPROC|RFMEM|RFNOWAIT|RFNOTEG)==0){
		fdread(1, s);
		exits(nil);
	}
	if(rfork(RFPROC|RFMEM|RFNOWAIT|RFNOTEG)==0){
		fdread(2, s);
		exits(nil);
	}
	fdinput(0, s);
	exits(nil);
}

void
fdread(int fd, Shell *s)
{
	char buf[8192+1];
	long n;

	while((n=read(s->fd[fd], buf, sizeof(buf)-1))>0){
		buf[n] = '\0';
		sleep(s->fddelay[fd]);
		if(write(fd, buf, n)!=n){
			fprint(2, "hubshell: write error copying on fd %d\n", fd);
			exits(nil);
		}
		if(s->shellctl == 'q')
			exits(nil);
	}
	if(n < 0)
		fprint(2, "hubshell: error reading fd %d\n", s->fd[fd]);
}

/* write user input from fd0 to hubfile */
void
fdinput(int fd, Shell *s)
{
	char buf[8192+1];
	long n;
	char ctlbuf[8192];
	int ctlfd;

readloop:
	while((n=read(fd, buf, sizeof(buf)-1))>0){
		buf[n] = '\0';
		/* check for user %command */
		if(buf[0] == '%'){
			s->cmdresult = 'p';
			parsebuf(s, buf + 1, s->fd[fd]);
			if(s->cmdresult != 'x'){
				memset(buf, 0, 8192);
				continue;
			}
		}
		sleep(s->fddelay[fd]);
		if(write(s->fd[fd], buf, n)!=n)
			fprint(2, "hubshell: write error copying on fd %d\n", s->fd[fd]);
		if(s->shellctl == 'q')
			exits(nil);
	}
	/* eof input from user, send message to hubfs ctl file */
	if(n == 0){
		if((ctlfd = open(ctlname, OWRITE)) < 0){
			fprint(2, "hubshell: can't open ctl file\n");
			goto readloop;
		}
		n = sprint(ctlbuf, "eof %s\n", basehub);
		if(write(ctlfd, ctlbuf, n) != n)
			fprint(2, "hubshell: error writing to %s on fd %d\n", ctlname, ctlfd);
		close(ctlfd);
	}
	/* hack to fix infinite loop bug with headless drawterm */
	if(n < 0)
		if(notereceived == 0)
			exits(nil);
	notereceived = 0;
	goto readloop;		/* Use more gotos, they aren't harmful */
}



/* close fds when a shell moves to new hubfs */
void
closefds(Shell *s)
{
	int i;

	for(i = 0; i < 3; i++)
		close(s->fd[i]);
}

/* handles %commands */
void
parsebuf(Shell *s, char *buf, int outfd)
{
	char *p, *q;
	char tmpstr[SMBUF];
	char tmpname[SMBUF];
	Shell *newshell;
	memset(tmpstr, 0, SMBUF);
	memset(tmpname, 0, SMBUF);
	char ctlbuf[SMBUF];
	int ctlfd;
	int cmd;
	long n;

	cmd = getcmd(buf);
	switch(cmd){
	case Detach:	/* %detach closes hubshell fds and exits */
		fprint(2, "hubshell: detaching\n");
		s->shellctl = 'q';
		if(fortunate)
			write(outfd, "fortune\n", 8);
		if(echoes)
			write(outfd, "echo\n", 5);
		closefds(s);
		exits(nil);
	case Remote:	/* %remote command makes new shell on hubfs host by sending hub -b command */
		if(isalpha(*(buf + 7)) == 0){
			fprint(2, "hubshell: remote needs a name parameter to create new hubs\n");
			break;
		}
		strncat(tmpname, buf + 7, strcspn(buf + 7, "\n"));
		fprint(2, "hubshell: creating new shell %s %s on remote host\n", srvname, tmpname);
		snprint(tmpstr, SMBUF, "hub -b %s %s\n", srvname, tmpname);
		write(outfd, tmpstr, strlen(tmpstr));
		snprint(tmpstr, SMBUF, "/n/%s/%s", srvname, tmpname);
		newshell = setupshell(tmpstr);
		if(newshell == nil){
			fprint(2, "hubshell: failed to setup up client shell, maybe problems on remote end\n");
			break;
		}
		s->shellctl = 'q';
		if(fortunate)
			write(outfd, "fortune\n", 8);
		if(echoes)
			write(outfd, "echo\n", 5);
		closefds(s);
		startshell(newshell);
	case Local:	/* %local command makes new shell on local machine by executing the hub command and exiting */
		if(isalpha(*(buf + 6)) == 0){
			fprint(2, "hubshell: local needs a name parameter to create new hubs\n");
			break;
		}
		strncpy(tmpstr, buf + 6, strcspn(buf + 6, "\n"));
		fprint(2, "hubshell: creating new local shell using hub %s %s\n", srvname, tmpstr);
		s->shellctl = 'q';
		if(fortunate)
			write(outfd, "fortune\n", 8);
		if(echoes)
			write(outfd, "echo\n", 5);
		closefds(s);
		execl("/bin/hub", "hub", srvname, tmpstr, 0);
		sysfatal("execl: %r");
	case Attach:	/* %attach name starts new shell and exits the current one */
		if(isalpha(*(buf + 7)) == 0){
			fprint(2, "hubshell: attach needs a name parameter to know what hubs to use, try %%list\n");
			break;
		}
		strncat(tmpname, buf + 7, strcspn(buf + 7, "\n"));
		fprint(2, "hubshell: attaching to  shell %s %s\n", srvname, tmpname);
		snprint(tmpstr, SMBUF, "/n/%s/%s", srvname, tmpname);
		newshell = setupshell(tmpstr);
		if(newshell == nil){
			fprint(2, "hubshell: failed to setup up client shell - do you need to create it with remote NAME?\n");
			break;
		}
		s->shellctl = 'q';
		if(fortunate)
			write(outfd, "fortune\n", 8);
		if(echoes)
			write(outfd, "echo\n", 5);
		closefds(s);
		startshell(newshell);
	case Err:	/* %err %in %out LONG set the delay before reading/writing on that fd to LONG milliseconds */
	case In:
	case Out:
		p = buf+strlen(cmdstr[cmd]);
		n = strtol(p, &q, 10);
		if(p == q || (*q != '\n' && *q != '\0')){
			fprint(2, "hubshell: %s hub delay setting requires numeric delay\n", cmdstr[cmd]);
			break;
		}
		s->fddelay[cmd-Err] = n;
		fprint(2, "hubshell: %s hub delay set to %ld\n", cmdstr[cmd], n);
		break;
	case Fortun:	/* %fortun and %echoes turn on buffer flush commands %unfort and %unecho deactivate */
		fprint(2, "hubshell: fortunes active\n");
		fortunate = 1;
		break;
	case Unfort:
		fprint(2, "hubshell: fortunes deactivated\n");
		fortunate = 0;
		break;
	case Echoes:
		fprint(2, "hubshell: echoes active\n");
		echoes = 1;
		break;
	case Unecho:
		fprint(2, "hubshell: echoes deactivated\n");
		echoes = 0;
		break;
	case Eof:	/* send eof or freeze/melt/fear/calm/trunc/notrunc messages to ctl file */
	case Freeze:
	case Melt:
	case Fear:
	case Calm:
	case Trunc:
	case Notrunc:
		if((ctlfd = open(ctlname, OWRITE)) < 0){
			fprint(2, "hubshell: can't open ctl file\n");
			break;
		}
		sprint(ctlbuf, "%s %s\n", cmdstr[cmd], basehub);
		n = strlen(ctlbuf);
		if(write(ctlfd, ctlbuf, n) != n)
			fprint(2, "hubshell: error writing to %s on fd %d\n", ctlname, ctlfd);
		close(ctlfd);
		break;
	case List:	/* %list displays attached hubs %status reports variable settings */
		sprint(tmpstr, "/n/%s", srvname);
		print("listing mounted hubfs at /n/%s\n", srvname);
		if(rfork(RFPROC|RFNOTEG|RFNOWAIT) == 0){
			execl("/bin/lc", "lc", tmpstr, 0);
			sysfatal("execl: %r");
		}
		break;
	case Status:
		print("\tHubshell attached to mounted %s of /srv/%s\n"
			"\tfdzero delay: %ld  fdone delay: %ld  fdtwo delay: %ld\n"
			, s->basename, srvname, s->fddelay[0], s->fddelay[1], s->fddelay[2]);
		if(fortunate)
			print("\tfortune fd flush active\n");
		if(echoes)
			print("\techo fd flush active\n");
		break;
	default:	/* no matching command found, print list of commands as reminder */
		fprint(2, "hubshell %% commands: \n\tdetach, remote NAME, local NAME, attach NAME \n\tstatus, list, err TIME, in TIME, out TIME\n\tfortun unfort echoes unecho trunc notrunc eof\n");
		s->cmdresult = 'x';
	}
	fprint(2, "io: ");
}

/* receive interrupt messages (delete key) and pass them through to attached shells */
int
sendinterrupt(void *regs, char *notename)
{
	char notehub[SMBUF];
	int notefd;

	if(strcmp(notename, "interrupt") != 0)
		return 0;
	notereceived = 1;
	if(regs == nil)		/* this is just to shut up a compiler warning */
		fprint(2, "error in note registers\n");
	sprint(notehub, "%s%s.note", hubdir, basehub);
	if((notefd = open(notehub, OWRITE)) < 0){
		fprint(2, "can't open %s\n", notehub);
		return 1;
	}
	fprint(notefd, "interrupt");
	close(notefd);
	return 1;
}

void
main(int argc, char *argv[])
{
	Shell *s;

	if(argc != 2){
		fprint(2, "usage: hubshell hubsname - and probably you want the hub wrapper script instead\n");
		exits("usage");
	}

	notereceived = 0;
	fortunate = 0;
	echoes = 0;		/* default changed to no echoes */

	/* parse initname and set srvname hubdir and ctlname from it */
	strncpy(initname, argv[1], SMBUF);
	strncat(srvname, initname+3, SMBUF);
	sprint(srvname + strcspn(srvname, "/"), "\0");
	sprint(hubdir, "/n/");
	strncat(hubdir, srvname, SMBUF-6);
	strcat(hubdir, "/");
	sprint(ctlname, "/n/");
	strncat(ctlname, srvname, SMBUF-6);
	strcat(ctlname, "/ctl");

	atnotify(sendinterrupt, 1);
	if((s = setupshell(initname)) == nil)
		sysfatal("couldnt setup shell, bailing out");
	startshell(s);
}
