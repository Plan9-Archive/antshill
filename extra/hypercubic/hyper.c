#include <u.h>
#include <libc.h>
#include <stdio.h>

char src[32], dest[32], curr[32];
int showports, showids, onehop, verbose, idq, adj;
long dim;

void
usage(void)
{
	fprint(2, "usage: hyper [-ipnov] -d dimension -f from -t to\n");
	exits("usage");
}

void
route(int dim)
{
	int i = 0;
	if(strncmp(curr, dest, dim) == 0){
		print("curr %s is dest %s\n", curr, dest);
		exits(nil);
	}
	while (i < (dim + 1)){
		if(curr[i] != dest[i]){
			if(verbose){
				print("At: %s	", curr);
				if(dest[i] == '1')
					print("moving forward via dimension %d\n", i);
				if(dest[i] == '0')
					print("moving backward via dimension %d\n", i);
			}
			curr[i] = dest[i];
			if(showports)
				print("%d\n",i);
			if(showids)
				print("%s\n", curr);
			if(onehop)
				exits(nil);
		}
		i++;
	}
	if(verbose)
		print("arrive at %s\n", curr);
}

void
idquery(int port)
{
	if(src[port] == '0'){
		src[port] = '1';
		printf("%s\n",src);
		exits(nil);
	}
	src[port] = '0';
	printf("%s\n",src);
	exits(nil);
}

void
adjacent()
{
	int i;
	for(i=0; i < dim; i++){
		strncpy(dest, src, dim);
		if(src[i]=='0'){
			dest[i]='1';
		} else {
			dest[i]='0';
		}
		print("%s\n",dest);
	}
	exits(nil);
}		

void
main(int argc, char *argv[])
{
	char *source="0000";
	char *destination="1111";
	char *dimension;
	dim = 4;
	showids = 1;

	ARGBEGIN{
	case 'a':
		adj = 1;
		break;
	case 'i':
		idq = 1;
		break;
	case 'v':
		verbose = 1;
		break;
	case 'p':
		showports = 1;
		break;
	case 'n':
		showids = 0;
		break;
	case 'o':
		onehop = 1;
		break;
	case 'f':
		source = EARGF(usage());
		break;
	case 't':
		destination = EARGF(usage());
		break;
	case 'd':
		dimension = EARGF(usage());
		dim = strtol(dimension, 0 , 10);
		break;
	default:
		usage();
	}ARGEND;
	if(idq){
		strncpy(src, source, 30);
		idquery(dim);
	}
	if(adj){
		strncpy(src, source, 30);
		dim=strlen(src);
		adjacent();
	}
	if(strlen(source) != strlen(destination))
		sysfatal("to and from values must be of equal length");
	if((strlen(source) != dim) || (strlen(destination) != dim))
		sysfatal("dimension doesnt macth input lengths");
	strncpy(src, source, dim);
	strncpy(dest, destination, dim);
	strncpy(curr, src, dim);
	if(verbose)
		print("routing in %ld dimensions from %s to %s\n",dim, src, dest);
	route(dim);
	exits(nil);
}

/* 
Given a site, there is a model category structure on the category
of cubical objects in the presheaf topos over that site, hence on the
category of cubical set-valued presheaves, such that at least the
corresponding homotopy category is equivalent to that of the
corresponding local model structure on simplicial presheaves (the
latter being known to present the (infinity,1)-topos over the given
site).  
*/
