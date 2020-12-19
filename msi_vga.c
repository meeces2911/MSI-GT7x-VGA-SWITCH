#include <getopt.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/reboot.h>

char *pname;

//const unsigned char NVRAM_OFFSET = 0x49;	// GT73VR with bios E17A1IMS.10A
//const unsigned char NVRAM_OFFSET = 0x4c;	// GT7x with bios 11D and(Or?) 122
const unsigned char NVRAM_OFFSET = 0x4d;	// GT72-2QE with bios E1781IMS.110

void
print_usage ()
{
  printf ("MSI VGA Select V0.2 by Zibri\n");
  printf ("http://www.zibri.org\n\n");
  printf ("Usage: %s%s -s -i -n -t -r\n",geteuid() != 0 ? "sudo ":"",pname);
  printf ("-s  Gets the name of the current VGA mode\n");
  printf ("-t  Toggles between the Nvidia and Intel VGA modes\n");
  printf ("-n  Sets the mode to Nvidia\n");
  printf ("-i  Sets the mode to Intel\n");
  printf ("-r  Automatically reboots the machine\n");
  exit (EXIT_FAILURE);
}

unsigned char getvga() 
{
    int fd1=-1;
    unsigned char data=NVRAM_OFFSET;
    unsigned char data2=0xff;

    fd1=open("/dev/port",O_RDWR|O_NDELAY);
    if ( fd1 < 0 ) print_usage ();;

    lseek(fd1,(0x70),SEEK_SET);
    write(fd1,&data,1);

    lseek(fd1,(0x71),SEEK_SET);
    read(fd1,&data2,1);


    return data2;

}
void setvga(unsigned char data2) 
{
    int fd1=-1;
    unsigned char data=NVRAM_OFFSET;

    fd1=open("/dev/port",O_RDWR|O_NDELAY);
    if ( fd1 < 0 ) print_usage ();;

    lseek(fd1,(0x70),SEEK_SET);
    write(fd1,&data,1);

    lseek(fd1,(0x71),SEEK_SET);
    write(fd1,&data2,1);

}

int set_mode(unsigned char new_mode)
{

  if (new_mode < 0x00 || new_mode > 0x01)
  {
    printf("Invalid/Unknown Mode selected. Aborting\n");
    exit(EXIT_FAILURE);
  }

  char* new_mode_str = (new_mode == 0x00) ? "Intel" : "Nvidia";
  unsigned char current = getvga();

  if (current == new_mode) {
    printf("Already running %s, nothing to do\n", new_mode_str);
    exit(EXIT_SUCCESS);
  }

  setvga(new_mode);

  if (getvga() != new_mode) {
    printf("Unable to change to %s VGA\n", new_mode_str);
    exit(EXIT_FAILURE);
  }

  printf("Successfully switched to: %s\n", new_mode_str);
  printf("Reboot required\n");
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
  int option = 0;
  pname=argv[0];

  unsigned char new_mode = 0x00;

  while ((option = getopt (argc, argv, "sintr")) != -1)
    {
      switch (option)
	{
	case 's':
	  /* Check/print what the current mode is */
          printf("%s\n",getvga() == 0x00 ? "Intel" : "Nvidia");
	  break;
	case 'i':
	  /* Set mode to Intel */
	  set_mode(0x00);
	  break;
	case 'n':
	  /* Set mode to Nvidia */
	  set_mode(0x01);
	  break;
	case 't':
	  /* Switch between Intel/Nvidia */
	  set_mode(getvga() ^ 0x01);
	  break;
	case 'r':
	  /* Reboot */
	  sync();
	  reboot(RB_AUTOBOOT);
    break;
	default:
	  print_usage (argv[0]);
	}
    }

  if (optind==1) print_usage (argv[0]);
  exit (EXIT_SUCCESS);
}

