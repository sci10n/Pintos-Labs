#include "filesys/inode.h"
#include <list.h>
#include <debug.h>
#include <round.h>
#include <string.h>
#include "filesys/filesys.h"
#include "filesys/free-map.h"
#include "threads/malloc.h"
#include "threads/synch.h"

#define inode_debug 0
/* Identifies an inode. */
#define INODE_MAGIC 0x494e4f44

/* On-disk inode.
   Must be exactly DISK_SECTOR_SIZE bytes long. */
struct inode_disk
{
  disk_sector_t start;                /* First data sector. */
  off_t length;                       /* File size in bytes. */
  unsigned magic;                     /* Magic number. */
  uint32_t unused[125];               /* Not used. */
};

/* Returns the number of sectors to allocate for an inode SIZE
   bytes long. */
static inline size_t
bytes_to_sectors (off_t size)
{
  return DIV_ROUND_UP (size, DISK_SECTOR_SIZE);
}

/* In-memory inode. */
struct inode
{
  struct list_elem elem;              /* Element in inode list. */
  disk_sector_t sector;               /* Sector number of disk location. */
  int open_cnt;                       /* Number of openers. */
  bool removed;                       /* True if deleted, false otherwise. */
  struct inode_disk data;             /* Inode content. */
  /*CHANGE*/ struct lock inode_lock;
  /*CHANGE*/ struct lock dir_lock;
  /*CHANGE*/ struct condition write_cond;
  /*CHANGE*/ struct condition read_cond;
  /*CHANGE*/ struct lock write_lock;
  /*CHANGE*/ struct lock read_lock;
  /*CHANGE*/ int write_count;
  /*CHANGE*/ int read_count;
};


/* Returns the disk sector that contains byte offset POS within
   INODE.
   Returns -1 if INODE does not contain data for a byte at offset
   POS. */
static disk_sector_t
byte_to_sector (const struct inode *inode, off_t pos)
{
  ASSERT (inode != NULL);
  if (pos < inode->data.length)
    return inode->data.start + pos / DISK_SECTOR_SIZE;
  else
    return -1;
}

/* List of open inodes, so that opening a single inode twice
   returns the same `struct inode'. */
static struct list open_inodes;

/*CHANGE*/static struct lock inode_list_lock;
/* Initializes the inode module. */
void
inode_init (void)
{
  list_init (&open_inodes);
  /*CHANGE*/
#if inode_debug
  debug("inode_init enter\n");
#endif
  /*CHANGE*/ lock_init(&(inode_list_lock));
  /*CHANGE*/
#if inode_debug
  debug("inode_init exit\n");
#endif
}

/* Initializes an inode with LENGTH bytes of data and
   writes the new inode to sector SECTOR on the file system
   disk.
   Returns true if successful.
   Returns false if memory or disk allocation fails. */
bool
inode_create (disk_sector_t sector, off_t length)
{
  struct inode_disk *disk_inode = NULL;
  bool success = false;
  /*CHANGE*/
#if inode_debug
  debug("inode_create enter\n");
#endif
  ASSERT (length >= 0);

  /* If this assertion fails, the inode structure is not exactly
     one sector in size, and you should fix that. */
  ASSERT (sizeof * disk_inode == DISK_SECTOR_SIZE);

  disk_inode = calloc (1, sizeof * disk_inode);
  if (disk_inode != NULL)
  {
    size_t sectors = bytes_to_sectors (length);
    disk_inode->length = length;
    disk_inode->magic = INODE_MAGIC;
    if (free_map_allocate (sectors, &disk_inode->start))
    {
      disk_write (filesys_disk, sector, disk_inode);
      if (sectors > 0)
      {
        static char zeros[DISK_SECTOR_SIZE];
        size_t i;

        for (i = 0; i < sectors; i++)
          disk_write (filesys_disk, disk_inode->start + i, zeros);
      }
      success = true;
    }
    free (disk_inode);
  }
  /*CHANGE*/
#if inode_debug
  debug("inode_create exit\n");
#endif
  return success;
}

/* Reads an inode from SECTOR
   and returns a `struct inode' that contains it.
   Returns a null pointer if memory allocation fails. */
struct inode *
inode_open (disk_sector_t sector)
{
  struct list_elem *e;
  struct inode *inode;
  /*CHANGE*/
#if inode_debug
  debug("inode_open enter\n");
#endif
  lock_acquire(&inode_list_lock);

  /* Check whether this inode is already open. */
  for (e = list_begin (&open_inodes); e != list_end (&open_inodes);
       e = list_next (e))
  {
    inode = list_entry (e, struct inode, elem);

    if (inode->sector == sector)
    {
      inode_reopen (inode);
      //lock_release(&(inode->dir_lock));
      lock_release(&inode_list_lock);
#if inode_debug
      debug("inode_open exit\n");
#endif
      return inode;
    }
  }

  /* Allocate memory. */
  inode = malloc (sizeof * inode);
  if (inode == NULL)
  {
    /*CHANGE*/
    lock_release(&inode_list_lock);
#if inode_debug
    debug("inode_open exit\n");
#endif
    return NULL;
  }

  list_push_front (&open_inodes, &inode->elem);
  /* Initialize. */
  inode->sector = sector;
  inode->open_cnt = 1;
  inode->removed = false;

  disk_read (filesys_disk, inode->sector, &inode->data);
  lock_init(&(inode->inode_lock));
  lock_init(&(inode->write_lock));
  lock_init(&(inode->read_lock));
  cond_init(&(inode->write_cond));
  cond_init(&(inode->read_cond));
  inode->write_count = 0;
  inode->read_count = 0;
  lock_release(&inode_list_lock);

  /*CHANGE*/
#if inode_debug
  debug("inode_open exit\n");
#endif
  return inode;
}

/* Reopens and returns INODE. */
struct inode *
inode_reopen (struct inode *inode)
{
  /*CHANGE*/
#if inode_debug
  debug("inode_reopen enter\n");
#endif
  if (inode != NULL)
  {
    lock_acquire(&(inode->inode_lock));
    inode->open_cnt++;
    lock_release(&(inode->inode_lock));
  }
  /*CHANGE*/
#if inode_debug
  debug("inode_reopen exit\n");
#endif
  return inode;
}

/* Returns INODE's inode number. */
disk_sector_t
inode_get_inumber (const struct inode *inode)
{
  /*CHANGE*/
#if inode_debug
  debug("inode_get_inumber enter\n");
#endif
  disk_sector_t tmp = inode->sector;
#if inode_debug
  debug("inode_get_inumber exit\n");
#endif
  return tmp;
}

/* Closes INODE and writes it to disk.
   If this was the last reference to INODE, frees its memory.
   If INODE was also a removed inode, frees its blocks. */
void
inode_close (struct inode *inode)
{
  /*CHANGE*/
#if inode_debug
  debug("inode_close enter\n");
#endif
  /* Ignore null pointer. */
  if (inode == NULL)
  {
    /*CHANGE*/
#if inode_debug
    debug("inode_close exit\n");
#endif
    return;
  }

  /*CHANGE*/ lock_acquire(&inode_list_lock);
  if (!lock_held_by_current_thread(&(inode->write_lock)))
    lock_acquire(&(inode->write_lock));
  lock_release(&(inode->write_lock));
  if (!lock_held_by_current_thread(&(inode->read_lock)))
    lock_acquire(&(inode->read_lock));
  lock_release(&(inode->read_lock));

  /*CHANGE*/ lock_acquire(&(inode->inode_lock));

  /* Release resources if this was the last opener. */
  if (--inode->open_cnt == 0)
  {
    /* Remove from inode list. */
    list_remove (&inode->elem);

    /* Deallocate blocks if the file is marked as removed. */
    if (inode->removed)
    {
      //lock_release(&(inode->dir_lock));
      free_map_release (inode->sector, 1);
      free_map_release (inode->data.start,
                        bytes_to_sectors (inode->data.length));
    }
    /*CHANGE*/

    lock_release(&(inode->inode_lock));
    free (inode);
    /*CHANGE*/ lock_release(&inode_list_lock);
    /*CHANGE*/
#if inode_debug
    debug("inode_close exit\n");
#endif
    return;
  }
  /*CHANGE*/ lock_release(&(inode->inode_lock));
  /*CHANGE*/ lock_release(&inode_list_lock);
  /*CHANGE*/
#if inode_debug
  debug("inode_close exit\n");
#endif
}

/* Marks INODE to be deleted when it is closed by the last caller who
   has it open. */
void
inode_remove (struct inode *inode)
{
  /*CHANGE*/
#if inode_debug
  debug("inode_remove enter\n");
#endif
  ASSERT (inode != NULL);
  inode->removed = true;
  /*CHANGE*/
#if inode_debug
  debug("inode_remove exit\n");
#endif
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
   Returns the number of bytes actually read, which may be less
   than SIZE if an error occurs or end of file is reached. */
off_t
inode_read_at (struct inode *inode, void *buffer_, off_t size, off_t offset)
{
  uint8_t *buffer = buffer_;
  off_t bytes_read = 0;
  uint8_t *bounce = NULL;
  /*CHANGE*/
#if inode_debug
  debug("inode_read_at enter\n");
#endif
  /*CHANGE*/
  /*  lock_acquire(&(inode->rw_check_lock));
    inode->read_count += 1;
    if (inode->write_count > 0)
    {
      debug("inside inode_read_at if write count = 0\n");
      lock_release(&(inode->rw_check_lock));
      lock_acquire(&(inode->read_lock));
      debug("inside inode_read_at if write count = 1\n");
      cond_wait(&(inode->read_cond), &(inode->read_lock));
      lock_release(&(inode->read_lock));
      debug("inside inode_read_at if write count = 2\n");
    }
    else
    {
      debug("inside inode_read_at if write count != 0\n");
      lock_release(&(inode->rw_check_lock));
    }*/
  lock_acquire(&(inode->inode_lock));
  ++(inode->read_count);
  if (inode->write_count != 0)
  {
    lock_release(&(inode->inode_lock));
    lock_acquire(&(inode->read_lock));
    cond_wait(&(inode->read_cond), &(inode->read_lock));
    lock_release(&(inode->read_lock));
  }
  else
  {
    lock_release(&(inode->inode_lock));
  }
  while (size > 0)
  {
    /* Disk sector to read, starting byte offset within sector. */
    disk_sector_t sector_idx = byte_to_sector (inode, offset);
    int sector_ofs = offset % DISK_SECTOR_SIZE;

    /* Bytes left in inode, bytes left in sector, lesser of the two. */
    off_t inode_left = inode_length (inode) - offset;
    int sector_left = DISK_SECTOR_SIZE - sector_ofs;
    int min_left = inode_left < sector_left ? inode_left : sector_left;

    /* Number of bytes to actually copy out of this sector. */
    int chunk_size = size < min_left ? size : min_left;
    if (chunk_size <= 0)
      break;

    if (sector_ofs == 0 && chunk_size == DISK_SECTOR_SIZE)
    {
      /* Read full sector directly into caller's buffer. */
      disk_read (filesys_disk, sector_idx, buffer + bytes_read);
    }
    else
    {
      /* Read sector into bounce buffer, then partially copy
         into caller's buffer. */
      if (bounce == NULL)
      {
        bounce = malloc (DISK_SECTOR_SIZE);
        if (bounce == NULL)
          break;
      }
      disk_read (filesys_disk, sector_idx, bounce);
      memcpy (buffer + bytes_read, bounce + sector_ofs, chunk_size);
    }

    /* Advance. */
    size -= chunk_size;
    offset += chunk_size;
    bytes_read += chunk_size;
  }
  free (bounce);
  /*CHANGE*/
  lock_acquire(&(inode->inode_lock));
  --(inode->read_count);
  if (inode->read_count == 0)
  {
    lock_acquire(&(inode->write_lock));
    cond_signal(&(inode->write_cond), &(inode->write_lock));
    lock_release(&(inode->write_lock));
  }
  lock_release(&(inode->inode_lock));
  /*  lock_acquire(&(inode->rw_check_lock));
    inode->read_count -= 1;
    if (inode->read_count == 0)
    {
      debug("inside inode_read_at if read count = 0\n");
      lock_release(&(inode->rw_check_lock));

      lock_acquire(&(inode->write_lock));
      cond_signal(&(inode->write_cond), &(inode->write_lock));
      lock_release(&(inode->write_lock));
    }
    else
    {
      debug("inside inode_read_at if read count != 0\n");
      lock_release(&(inode->rw_check_lock));
    }*/

  /*CHANGE*/
#if inode_debug
  debug("inode_read_at exit\n");
#endif
  return bytes_read;
}

/* Writes SIZE bytes from BUFFER into INODE, starting at OFFSET.
   Returns the number of bytes actually written, which may be
   less than SIZE if end of file is reached or an error occurs.
   (Normally a write at end of file would extend the inode, but
   growth is not yet implemented.) */
off_t
inode_write_at (struct inode *inode, const void *buffer_, off_t size,
                off_t offset)
{
  const uint8_t *buffer = buffer_;
  off_t bytes_written = 0;
  uint8_t *bounce = NULL;
  /*CHANGE*/
#if inode_debug
  debug("inode_write_at enter\n");
#endif

  /*CHANGE*/
  /*  lock_acquire(&(inode->rw_check_lock));
    inode->write_count += 1;
    if (inode->write_count > 1 || inode->read_count > 0)
    {
      debug("Thread enter in wait write: %i, %i\n", inode->write_count, inode->read_count);
      lock_release(&(inode->rw_check_lock));
      lock_acquire(&(inode->write_lock));

      cond_wait(&(inode->write_cond), &(inode->write_lock));
    }
    else
    {
      lock_release(&(inode->rw_check_lock));
      lock_acquire(&(inode->write_lock));
    }*/
  lock_acquire(&(inode->inode_lock));
  ++(inode->write_count);
  if (inode->write_count != 1 || inode->read_count != 0)
  {
    lock_release(&(inode->inode_lock));
    lock_acquire(&(inode->write_lock));
    cond_wait(&(inode->write_cond), &(inode->write_lock));
  }
  else
  {
    lock_acquire(&(inode->write_lock));
    lock_release(&(inode->inode_lock));
  }
  lock_acquire(&(inode->read_lock));


  while (size > 0)
  {
    /* Sector to write, starting byte offset within sector. */
    disk_sector_t sector_idx = byte_to_sector (inode, offset);
    int sector_ofs = offset % DISK_SECTOR_SIZE;

    /* Bytes left in inode, bytes left in sector, lesser of the two. */
    off_t inode_left = inode_length (inode) - offset;
    int sector_left = DISK_SECTOR_SIZE - sector_ofs;
    int min_left = inode_left < sector_left ? inode_left : sector_left;

    /* Number of bytes to actually write into this sector. */
    int chunk_size = size < min_left ? size : min_left;
    if (chunk_size <= 0)
      break;

    if (sector_ofs == 0 && chunk_size == DISK_SECTOR_SIZE)
    {
      /* Write full sector directly to disk. */
      disk_write (filesys_disk, sector_idx, buffer + bytes_written);
    }
    else
    {
      /* We need a bounce buffer. */
      if (bounce == NULL)
      {
        bounce = malloc (DISK_SECTOR_SIZE);
        if (bounce == NULL)
          break;
      }

      /* If the sector contains data before or after the chunk
         we're writing, then we need to read in the sector
         first.  Otherwise we start with a sector of all zeros. */
      if (sector_ofs > 0 || chunk_size < sector_left)
        disk_read (filesys_disk, sector_idx, bounce);
      else
        memset (bounce, 0, DISK_SECTOR_SIZE);
      memcpy (bounce + sector_ofs, buffer + bytes_written, chunk_size);
      disk_write (filesys_disk, sector_idx, bounce);
    }

    /* Advance. */
    size -= chunk_size;
    offset += chunk_size;
    bytes_written += chunk_size;
  }
  free (bounce);
  lock_acquire(&(inode->inode_lock));
  --(inode->write_count);
  if (inode->write_count != 0)
  {
    cond_signal(&(inode->write_cond), &(inode->write_lock));
  }
  else
  {
    cond_broadcast(&(inode->read_cond), &(inode->read_lock));
  }
  lock_release(&(inode->read_lock));
  lock_release(&(inode->write_lock));
  lock_release(&(inode->inode_lock));
  /*  lock_acquire(&(inode->rw_check_lock));
    inode->write_count -= 1;
    if (inode->write_count > 0)
    {
      debug("Thread enter in signal write: %i\n", inode->write_count);
      cond_signal(&(inode->write_cond), &(inode->write_lock));
      lock_release(&(inode->write_lock));
      lock_release(&(inode->rw_check_lock));
    }
    else
    {
    debug("Thread enter in broadcast write: %i\n", inode->write_count);
      lock_release(&(inode->write_lock));
      lock_release(&(inode->rw_check_lock));
      lock_acquire(&(inode->read_lock));
      cond_broadcast(&(inode->read_cond), &(inode->read_lock));
      lock_release(&(inode->read_lock));
       debug("Thread enter in broadcast write done: %i\n", inode->write_count);
    }*/


#if inode_debug
  debug("inode_write_at exit\n");
#endif
  return bytes_written;
}

/* Returns the length, in bytes, of INODE's data. */
off_t
inode_length (const struct inode *inode)
{
  return inode->data.length;
}

void inode_dir_lock(struct inode * i)
{
#if inode_debug
  debug("inode_dir_lock enter\n");
#endif
  if (i != NULL)
    lock_acquire(&(i->dir_lock));
#if inode_debug
  debug("inode_dir_lock exit\n");
#endif
}

void inode_dir_unlock(struct inode * i)
{
#if inode_debug
  debug("inode_dir_unlock enter\n");
#endif
  if (i != NULL)
    lock_release(&(i->dir_lock));
#if inode_debug
  debug("inode_dir_lock exit\n");
#endif
}

void inode_dir_init(struct inode * i)
{
#if inode_debug
  debug("inode_dir_init enter\n");
#endif
  if (i != NULL)
    lock_init(&(i->dir_lock));
#if inode_debug
  debug("inode_dir_init exit\n");
#endif
}
