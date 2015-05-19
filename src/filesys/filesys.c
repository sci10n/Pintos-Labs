#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "devices/disk.h"
#include "threads/synch.h"

#define filesys_debug 0

/* The disk that contains the file system. */
struct disk *filesys_disk;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format)
{
  /*CHANGE*/
#if filesys_debug
  debug("filesys_init enter\n");
#endif
  filesys_disk = disk_get (0, 1);
  if (filesys_disk == NULL)
    PANIC ("hd0:1 (hdb) not present, file system initialization failed");

  inode_init ();
  free_map_init ();

  if (format)
    do_format ();

  free_map_open ();
  /*CHANGE*/
#if filesys_debug
  debug("filesys_init exit\n");
#endif
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void)
{
  /*CHANGE*/
#if filesys_debug
  debug("filesys_done enter\n");
#endif
  free_map_close ();
  /*CHANGE*/
#if filesys_debug
  debug("filesys_done exit\n");
#endif
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size)
{
  /*CHANGE*/
#if filesys_debug
  debug("filesys_create enter\n");
#endif
  disk_sector_t inode_sector = 0;
  struct dir *dir = dir_open_root ();
  bool success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size)
                  && dir_add (dir, name, inode_sector));
  if (!success && inode_sector != 0)
    free_map_release (inode_sector, 1);
  dir_close (dir);
  /*CHANGE*/
#if filesys_debug
  debug("filesys_create exit\n");
#endif
  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
  /*CHANGE*/
#if filesys_debug
  debug("filesys_open enter\n");
#endif
  struct dir *dir = dir_open_root ();
  struct inode *inode = NULL;
  struct file *file = NULL;

  // TODO: LOCK DIR

  if (dir != NULL)
    {
      dir_lookup (dir, name, &inode);
    }

  dir_close (dir);

  file = file_open (inode);
  /*CHANGE*/
#if filesys_debug
  debug("filesys_open exit\n");
#endif
  return file;
}

void
filesys_close (struct file *file)
{
  /*CHANGE*/
#if filesys_debug
  debug("filesys_close enter\n");
#endif
  file_close(file);
  /*CHANGE*/
#if filesys_debug
  debug("filesys_close exit\n");
#endif
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name)
{
  /*CHANGE*/
#if filesys_debug
  debug("filesys_remove enter\n");
#endif
  struct dir *dir = dir_open_root ();
  bool success = dir != NULL && dir_remove (dir, name);
  dir_close (dir);
  /*CHANGE*/
#if filesys_debug
  debug("filesys_remove exit\n");
#endif
  return success;
}

/* Formats the file system. */
static void
do_format (void)
{
  /*CHANGE*/
#if filesys_debug
  debug("do_format enter\n");
#endif
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
  /*CHANGE*/
#if filesys_debug
  debug("do_format exit\n");
#endif
}
