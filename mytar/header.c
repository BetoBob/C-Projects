#include "mytar.h"

/* functions to convert file
stats to header contents of .tar file */

void headerName(char * inputfile, header * head, struct stat ifile)
{
  	int c;
	memset(head -> name, '\0', 100);
	memset(head -> prefix, '\0', 155);

	if (S_ISDIR(ifile.st_mode))
		strcat(inputfile, "/");

	for (c = 0; c < 100; c++)
    {
    	if (c < strlen(inputfile))
    		head -> name[c] = inputfile[c];
    }
    for (; c < 255; c++)
    {
    	if (c < strlen(inputfile))
    		head -> prefix[c-100] = inputfile[c];
    }

	if (S_ISDIR(ifile.st_mode))
		inputfile[strlen(inputfile)-1] = 0;
}

void headerMode(struct stat ifile, uint8_t *buffer)
{
  memset(buffer, '\0', 8);
  sprintf(buffer, "%07o", ifile.st_mode & 0777);
}

void headerUID(struct stat ifile, uint8_t *buffer)
{
  memset(buffer, '\0', 8);

  sprintf(buffer, "%07o", ifile.st_uid);
}

void headerGID(struct stat ifile, uint8_t *buffer)
{
  memset(buffer, '\0', 8);

  sprintf(buffer, "%07o", ifile.st_gid);

}

void headerSize(struct stat ifile, uint8_t *buffer)
{
  memset(buffer, '\0', 12);

  sprintf(buffer, "%011o", (unsigned int) ifile.st_size);
}

void headerMTime(struct stat ifile, uint8_t *buffer)
{
  memset(buffer, '\0', 12);

  sprintf(buffer, "%011o", (unsigned int) ifile.st_mtim.tv_sec);
}

void headerChkSum(header *head, uint8_t *buffer)
{
  int c, sum = 0;
  bool isChkSum;
  uint8_t *headerArr = (uint8_t *) head;

  memset(buffer, '\0', 8);

  for (c = 0; c < 512; c++)
  {
    isChkSum = ((c >= 148) && (c < 156));
    sum += (isChkSum) ? ' ' : headerArr[c];
  }

  sprintf(buffer, "%07o", sum);
}

void headerTypeFlag(struct stat ifile, uint8_t *buffer)
{
  if (S_ISDIR(ifile.st_mode))
  {
    *buffer = '5';
  }
  else if (S_ISLNK(ifile.st_mode))
  {
    *buffer = '2';
  }
  else
  {
    *buffer = '0';
  }
}

void headerLinkName(struct stat ifile, char *pathname, uint8_t *buffer)
{
  memset(buffer, '\0', sizeof(uint8_t)*100);

  if (S_ISLNK(ifile.st_mode))
  {
    if (-1 != (readlink(pathname, buffer, 100)))
    {
      perror("Cannot read symbolic link");
      exit(-1);
    }
  }
}

void headerUname(struct stat ifile, uint8_t *buffer)
{
  struct passwd *user;

  memset(buffer, '\0', 32);

  if (0 == (user = getpwuid(ifile.st_uid)))
  {
    perror("Cannot get user name");
    exit(-1);
  }

  sprintf(buffer, "%s", user -> pw_name);
}

void headerGname(struct stat ifile, uint8_t *buffer)
{
  struct group *group;

  memset(buffer, '\0', 32);

  if (0 == (group = getgrgid(ifile.st_gid)))
  {
    perror("Cannot get group name");
    exit(-1);
  }

  sprintf(buffer, "%s", group -> gr_name);
}

void headerDevmajor(struct stat ifile, uint8_t *buffer)
{
	memset(buffer, '\0', 8);
	/*
	if (!S_ISREG(ifile.st_mode))
		sprintf(buffer, "%7o", major(ifile.st_dev));
	*/
}

void headerDevminor(struct stat ifile, uint8_t *buffer)
{
	memset(buffer, '\0', 8);
	/*
	if (!S_ISREG(ifile.st_mode))
		sprintf(buffer, "%7o", minor(ifile.st_dev)); */
}
