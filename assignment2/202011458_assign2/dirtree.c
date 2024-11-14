//--------------------------------------------------------------------------------------------------
// System Programming                         I/O Lab                                     Fall 2024
//
/// @file
/// @brief resursively traverse directory tree and list all entries
/// @author <KimSungJin>
/// @studid <2020-11458>
//--------------------------------------------------------------------------------------------------

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <assert.h>
#include <grp.h>
#include <pwd.h>

#define MAX_DIR 64            ///< maximum number of supported directories

/// @brief output control flags
#define F_TREE      0x1       ///< enable tree view
#define F_SUMMARY   0x2       ///< enable summary
#define F_VERBOSE   0x4       ///< turn on verbose mode

/// @brief struct holding the summary
struct summary {
  unsigned int dirs;          ///< number of directories encountered
  unsigned int files;         ///< number of files
  unsigned int links;         ///< number of links
  unsigned int fifos;         ///< number of pipes
  unsigned int socks;         ///< number of sockets

  unsigned long long size;    ///< total size (in bytes)
  unsigned long long blocks;  ///< total number of blocks (512 byte blocks)
};


/// @brief abort the program with EXIT_FAILURE and an optional error message
///
/// @param msg optional error message or NULL
void panic(const char *msg)
{
  if (msg) fprintf(stderr, "%s\n", msg);
  exit(EXIT_FAILURE);
}


/// @brief read next directory entry from open directory 'dir'. Ignores '.' and '..' entries
///
/// @param dir open DIR* stream
/// @retval entry on success
/// @retval NULL on error or if there are no more entries
struct dirent *getNext(DIR *dir)
{
  struct dirent *next;
  int ignore;

  do {
    errno = 0;
    next = readdir(dir);
    if (errno != 0) perror(NULL);
    ignore = next && ((strcmp(next->d_name, ".") == 0) || (strcmp(next->d_name, "..") == 0));
  } while (next && ignore);

  return next;
}


/// @brief qsort comparator to sort directory entries. Sorted by name, directories first.
///
/// @param a pointer to first entry
/// @param b pointer to second entry
/// @retval -1 if a<b
/// @retval 0  if a==b
/// @retval 1  if a>b
static int dirent_compare(const void *a, const void *b)
{
  struct dirent *e1 = (struct dirent*)a;
  struct dirent *e2 = (struct dirent*)b;

  // if one of the entries is a directory, it comes first
  if (e1->d_type != e2->d_type) {
    if (e1->d_type == DT_DIR) return -1;
    if (e2->d_type == DT_DIR) return 1;
  }

  // otherwise sorty by name
  return strcmp(e1->d_name, e2->d_name);
}


/// @brief recursively process directory @a dn and print its tree
///
/// @param dn absolute or relative path string
/// @param pstr prefix string printed in front of each entry
/// @param stats pointer to statistics
/// @param flags output control flags (F_*)
void processDir(const char *dn, const char *pstr, struct summary *stats, unsigned int flags)
{
  DIR* dp = NULL;
  struct dirent* ent = NULL;
  struct stat buf;
  int entry_count = 0;

  if((dp = opendir(dn)) == NULL){ // count the entries in dn
    if(flags & F_TREE){ // tree view
      printf("%s`-ERROR: %s\n", pstr, strerror(errno));
    }
    else{
      printf("%s  ERROR: %s\n", pstr, strerror(errno));
    }
    return;
  }
  while((ent = getNext(dp)) != NULL){
    entry_count += 1;
  }
  closedir(dp);

  struct dirent entry[entry_count];
  int cnt = 0;

  dp = opendir(dn);
  while((ent = getNext(dp)) != NULL){ // put each entry into array
    entry[cnt++] = *ent;
  }
  qsort(entry, entry_count, sizeof(struct dirent), dirent_compare);

  for(int i=0; i<entry_count; i++){ // loop for each entry
    struct dirent* tmp = &entry[i];

    // if entry name is too long, cut end and with three dots
    char entry_name[55];
    int name_len = strlen(pstr) + 2 + strlen(tmp->d_name);
    if(name_len > 54){
      strncpy(entry_name, tmp->d_name, 49-strlen(pstr));
      entry_name[49-strlen(pstr)] = '\0';
      strcat(entry_name, "...");
    }
    else{
      strncpy(entry_name, tmp->d_name, sizeof(entry_name));
      entry_name[sizeof(entry_name) - 1] = '\0';
    }
    int entry_name_len = strlen(entry_name) + strlen(pstr);

    char f_path[strlen(dn) + 2 + strlen(tmp->d_name)]; // full path of the entry
    snprintf(f_path, sizeof(f_path), "%s/%s", dn, tmp->d_name);
    if(lstat(f_path, &buf) == -1){ // print error when an error found
      if(flags & F_TREE){ // tree view
        if(i == entry_count-1){
          printf("%s`-%s", pstr, entry_name);
        }
        else{
          printf("%s|-%s", pstr, entry_name);
        }
      }
      else{ //simple mode
        printf("%s  %s", pstr, entry_name);
      }
      for(int j=0; j<64-entry_name_len-10; j++){
        printf(" ");
      }
      printf("%s\n", strerror(errno));
      continue;
    }

    char type = ' ';
    if(S_ISREG(buf.st_mode)) stats->files += 1;
    else if(S_ISLNK(buf.st_mode)){
      stats->links += 1;
      type = 'l';
    }
    else if(S_ISFIFO(buf.st_mode)){
      stats->fifos += 1;
      type = 'f';
    }
    else if(S_ISSOCK(buf.st_mode)){
      stats->socks += 1;
      type = 's';
    }
    else if(S_ISDIR(buf.st_mode)){
      stats->dirs += 1;
      type = 'd';
    }
    else if(S_ISBLK(buf.st_mode)){
      type = 'b';
    }
    else if(S_ISCHR(buf.st_mode)){
      type = 'c';
    }
    stats->size += buf.st_size;
    stats->blocks += buf.st_blocks;

    if(flags & F_TREE){ // tree view
      if(i == entry_count-1){
        printf("%s`-%s", pstr, entry_name);
      }
      else{
        printf("%s|-%s", pstr, entry_name);
      }

      if(flags & F_VERBOSE){ // verbose
        struct passwd *pw = getpwuid(buf.st_uid);
        struct group *gr = getgrgid(buf.st_gid);
        for(int j=0; j<64-entry_name_len-10; j++){ // space between Name and UserID
          printf(" ");
        }
        char size[11];
        char blocks[9];
        sprintf(size, "%ld", buf.st_size);
        sprintf(blocks, "%ld", buf.st_blocks);
        printf("%8.8s:%-8.8s  %10.10s  %8.8s  %c", pw->pw_name, gr->gr_name, size, blocks, type);
      }
      printf("\n");
      
      if(S_ISDIR(buf.st_mode)){ // if the entry is a directory, recursively process dir
        char *new_dn = (char*)malloc(strlen(dn) + strlen(tmp->d_name) + 2);
        strcpy(new_dn, dn);
        strcat(new_dn, "/");
        strcat(new_dn, tmp->d_name);

        char *new_pstr = (char*)malloc(strlen(pstr) + 3);
        strcpy(new_pstr, pstr);
        if(i == entry_count-1) strcat(new_pstr, "  ");
        else strcat(new_pstr, "| ");

        processDir(new_dn, strcmp(pstr, "") ? new_pstr : "| ", stats, flags);
        free(new_dn);
        free(new_pstr);
      }
    }
    else{ // simple mode
      printf("%s  %s", pstr, entry_name);

      if(flags & F_VERBOSE){ // verbose
        struct passwd *pw = getpwuid(buf.st_uid);
        struct group *gr = getgrgid(buf.st_gid);
        for(int j=0; j<64-entry_name_len-10; j++){
          printf(" ");
        }
        char size[11];
        char blocks[9];
        sprintf(size, "%ld", buf.st_size);
        sprintf(blocks, "%ld", buf.st_blocks);
        printf("%8.8s:%-8.8s  %10.10s  %8.8s  %c", pw->pw_name, gr->gr_name, size, blocks, type);
      }
      printf("\n");

      if(S_ISDIR(buf.st_mode)){ // if the entry is a directory, recursively process dir
        char *new_dn = (char*)malloc(strlen(dn) + strlen(tmp->d_name) + 2);
        strcpy(new_dn, dn);
        strcat(new_dn, "/");
        strcat(new_dn, tmp->d_name);

        char *new_pstr = (char*)malloc(strlen(pstr) + 3);
        strcpy(new_pstr, pstr);
        strcat(new_pstr, "  ");

        processDir(new_dn, new_pstr, stats, flags);
        free(new_dn);
        free(new_pstr);
      }
    }
  }
  closedir(dp);
}


/// @brief print program syntax and an optional error message. Aborts the program with EXIT_FAILURE
///
/// @param argv0 command line argument 0 (executable)
/// @param error optional error (format) string (printf format) or NULL
/// @param ... parameter to the error format string
void syntax(const char *argv0, const char *error, ...)
{
  if (error) {
    va_list ap;

    va_start(ap, error);
    vfprintf(stderr, error, ap);
    va_end(ap);

    printf("\n\n");
  }

  assert(argv0 != NULL);

  fprintf(stderr, "Usage %s [-t] [-s] [-v] [-h] [path...]\n"
                  "Gather information about directory trees. If no path is given, the current directory\n"
                  "is analyzed.\n"
                  "\n"
                  "Options:\n"
                  " -t        print the directory tree (default if no other option specified)\n"
                  " -s        print summary of directories (total number of files, total file size, etc)\n"
                  " -v        print detailed information for each file. Turns on tree view.\n"
                  " -h        print this help\n"
                  " path...   list of space-separated paths (max %d). Default is the current directory.\n",
                  basename(argv0), MAX_DIR);

  exit(EXIT_FAILURE);
}


/// @brief program entry point
int main(int argc, char *argv[])
{
  //
  // default directory is the current directory (".")
  //
  const char CURDIR[] = ".";
  const char *directories[MAX_DIR];
  int   ndir = 0;

  struct summary tstat;
  unsigned int flags = 0;

  //
  // parse arguments
  //
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      // format: "-<flag>"
      if      (!strcmp(argv[i], "-t")) flags |= F_TREE;
      else if (!strcmp(argv[i], "-s")) flags |= F_SUMMARY;
      else if (!strcmp(argv[i], "-v")) flags |= F_VERBOSE;
      else if (!strcmp(argv[i], "-h")) syntax(argv[0], NULL);
      else syntax(argv[0], "Unrecognized option '%s'.", argv[i]);
    } else {
      // anything else is recognized as a directory
      if (ndir < MAX_DIR) {
        directories[ndir++] = argv[i];
      } else {
        printf("Warning: maximum number of directories exceeded, ignoring '%s'.\n", argv[i]);
      }
    }
  }

  // if no directory was specified, use the current directory
  if (ndir == 0) directories[ndir++] = CURDIR;


  //
  // process each directory
  //
  // TODO
  //
  // Pseudo-code
  // - reset statistics (tstat)
  // - loop over all entries in 'directories' (number of entires stored in 'ndir')
  //   - reset statistics (dstat)
  //   - if F_SUMMARY flag set: print header
  //   - print directory name
  //   - call processDir() for the directory
  //   - if F_SUMMARY flag set: print summary & update statistics

  memset(&tstat, 0, sizeof(tstat));

  for(int i=0; i<ndir; i++){
    struct summary dstat;
    memset(&dstat, 0, sizeof(dstat));

    if(flags & (1 << 1)){ // summary
      if(flags & (1 << 2)){ // verbose
        printf("Name                                                        User:Group           Size    Blocks Type \n");
      }
      else{
        printf("Name\n");
      }
      printf("----------------------------------------------------------------------------------------------------\n");
    }

    // if directory name is too long, cut end and with three dots
    char dir_name[55];
    if(strlen(directories[i]) > 54){
      strncpy(dir_name, directories[i], 51);
      dir_name[51] = '\0';
      strcat(dir_name, "...");
    }
    else{
      strncpy(dir_name, directories[i], sizeof(dir_name));
      dir_name[sizeof(dir_name) - 1] = '\0';
    }
    printf("%s\n", dir_name);

    processDir(directories[i], "", &dstat, flags);

    if(flags & F_SUMMARY){ // summary
      printf("----------------------------------------------------------------------------------------------------\n");

      char *txt = (char*)malloc(70);
      txt[0] = '\0';
      char *tmp = (char*)malloc(20);

      sprintf(tmp, "%d", dstat.files);
      if(dstat.files == 1) strcat(strcat(txt, tmp), " file, ");
      else strcat(strcat(txt, tmp), " files, ");

      sprintf(tmp, "%d", dstat.dirs);
      if(dstat.dirs == 1) strcat(strcat(txt, tmp), " directory, ");
      else strcat(strcat(txt, tmp), " directories, ");

      sprintf(tmp, "%d", dstat.links);
      if(dstat.links == 1) strcat(strcat(txt, tmp), " link, ");
      else strcat(strcat(txt, tmp), " links, ");

      sprintf(tmp, "%d", dstat.fifos);
      if(dstat.fifos == 1) strcat(strcat(txt, tmp), " pipe, ");
      else strcat(strcat(txt, tmp), " pipes, ");

      strcat(txt, "and ");

      sprintf(tmp, "%d", dstat.socks);
      if(dstat.socks == 1) strcat(strcat(txt, tmp), " socket");
      else strcat(strcat(txt, tmp), " sockets");

      // if summary is too long, cut end and with three dots
      char txt_buf[69];
      int txt_len = strlen(txt);
      if(txt_len > 68){
        strncpy(txt_buf, txt, 65);
        txt_buf[65] = '\0';
        strcat(txt_buf, "...");
      }
      else{
        strncpy(txt_buf, txt, sizeof(txt_buf));
        txt_buf[sizeof(txt_buf) - 1] = '\0';
      }
      printf("%-68.68s", txt_buf);
      free(txt);
      
      if(flags & F_VERBOSE){ // verbose
        char tot_size[15];
        char tot_blocks[10];
        sprintf(tot_size, "%lld", dstat.size);
        sprintf(tot_blocks, "%lld", dstat.blocks);
        printf("   %14.14s %9.9s", tot_size, tot_blocks);
      }

      printf("\n");
      printf("\n");
      free(tmp);

      tstat.files += dstat.files;
      tstat.dirs += dstat.dirs;
      tstat.links += dstat.links;
      tstat.fifos += dstat.fifos;
      tstat.socks += dstat.socks;
      tstat.size += dstat.size;
      tstat.blocks += dstat.blocks;
    }
  }
  //...


  //
  // print grand total
  //
  if ((flags & F_SUMMARY) && (ndir > 1)) {
    printf("Analyzed %d directories:\n"
           "  total # of files:        %16d\n"
           "  total # of directories:  %16d\n"
           "  total # of links:        %16d\n"
           "  total # of pipes:        %16d\n"
           "  total # of sockets:      %16d\n",
           ndir, tstat.files, tstat.dirs, tstat.links, tstat.fifos, tstat.socks);

    if (flags & F_VERBOSE) {
      printf("  total file size:         %16llu\n"
             "  total # of blocks:       %16llu\n",
             tstat.size, tstat.blocks);
    }

  }

  //
  // that's all, folks!
  //
  return EXIT_SUCCESS;
}

