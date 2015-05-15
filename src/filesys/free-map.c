#include "filesys/free-map.h"
#include <bitmap.h>
#include <debug.h>
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/inode.h"
#include "threads/synch.h"

#define free_map_debug 0

static struct file *free_map_file;   /* Free map file. */
static struct bitmap *free_map;      /* Free map, one bit per disk sector. */

struct lock free_map_lock;

/* Initializes the free map. */
void
free_map_init (void)
{
  //CHANGE//
#if free_map_debug
  debug("free_map_init enter\n");
#endif
  free_map = bitmap_create (disk_size (filesys_disk));
  if (free_map == NULL)
    PANIC ("bitmap creation failed--disk is too large");
  bitmap_mark (free_map, FREE_MAP_SECTOR);
  bitmap_mark (free_map, ROOT_DIR_SECTOR);

  lock_init(&free_map_lock);
#if free_map_debug
  debug("free_map_init exit\n");
#endif
}

/* Allocates CNT consecutive sectors from the free map and stores
   the first into *SECTORP.
   Returns true if successful, false if all sectors were
   available. */
bool
free_map_allocate (size_t cnt, disk_sector_t *sectorp)
{
  disk_sector_t sector;

  //CHANGE//
#if free_map_debug
  debug("free_map_allocate enter\n");
#endif
  //lock_acquire(&free_map_lock);

  sector = bitmap_scan_and_flip (free_map, 0, cnt, false);
  if (sector != BITMAP_ERROR
      && free_map_file != NULL
      && !bitmap_write (free_map, free_map_file))
  {
    bitmap_set_multiple (free_map, sector, cnt, false);
    sector = BITMAP_ERROR;
  }
  //lock_release(&free_map_lock);


  if (sector != BITMAP_ERROR)
    *sectorp = sector;

  //CHANGE//
#if free_map_debug
  debug("free_map_allocate exit\n");
#endif
  return sector != BITMAP_ERROR;
}

/* Makes CNT sectors starting at SECTOR available for use. */
void
free_map_release (disk_sector_t sector, size_t cnt)
{
  ASSERT (bitmap_all (free_map, sector, cnt));
  //CHANGE//
#if free_map_debug
  debug("free_map_release enter\n");
#endif
  lock_acquire(&free_map_lock);
  bitmap_set_multiple (free_map, sector, cnt, false);
  bitmap_write (free_map, free_map_file);
  lock_release(&free_map_lock);
  ////CHANGE// lock_release(&free_map_lock);

  //CHANGE//
#if free_map_debug
  debug("free_map_release exit\n");
#endif
}

/* Opens the free map file and reads it from disk. */
void
free_map_open (void)
{
  //CHANGE//
#if free_map_debug
  debug("free_map_open enter\n");
#endif
  free_map_file = file_open (inode_open (FREE_MAP_SECTOR));
  if (free_map_file == NULL)
    PANIC ("can't open free map");
// //CHANGE// lock_acquire(&free_map_lock);
    lock_acquire(&free_map_lock);
  if (!bitmap_read (free_map, free_map_file))
    PANIC ("can't read free map");
  lock_release(&free_map_lock);
//  //CHANGE// lock_release(&free_map_lock);
  //CHANGE//
#if free_map_debug
  debug("free_map_open exit\n");
#endif
}

/* Writes the free map to disk and closes the free map file. */
void
free_map_close (void)
{
  //CHANGE//
#if free_map_debug
  debug("free_map_close enter\n");
#endif
  file_close (free_map_file);
#if free_map_debug
  debug("free_map_close exit\n");
#endif
}

/* Creates a new free map file on disk and writes the free map to
   it. */
void
free_map_create (void)
{
  //CHANGE//
#if free_map_debug
  debug("free_map_create enter\n");
#endif
//  //CHANGE// lock_acquire(&free_map_lock);
  lock_acquire(&free_map_lock);
  /* Create inode. */
  if (!inode_create (FREE_MAP_SECTOR, bitmap_file_size (free_map)))
    PANIC ("free map creation failed");

  /* Write bitmap to file. */
  free_map_file = file_open (inode_open (FREE_MAP_SECTOR));
  if (free_map_file == NULL)
    PANIC ("can't open free map");
  if (!bitmap_write (free_map, free_map_file))
    PANIC ("can't write free map");
  //CHANGE// lock_release(&free_map_lock);
  //CHANGE//
  lock_release(&free_map_lock);
#if free_map_debug
  debug("free_map_create exit\n");
#endif
}
