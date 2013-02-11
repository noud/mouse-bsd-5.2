/*	$NetBSD: ext2fs_extern.h,v 1.39.6.1 2009/10/27 21:41:07 bouyer Exp $	*/

/*-
 * Copyright (c) 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ffs_extern.h	8.3 (Berkeley) 4/16/94
 * Modified for ext2fs by Manuel Bouyer.
 */

/*-
 * Copyright (c) 1997 Manuel Bouyer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by Manuel Bouyer.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	@(#)ffs_extern.h	8.3 (Berkeley) 4/16/94
 * Modified for ext2fs by Manuel Bouyer.
 */

#ifndef _UFS_EXT2FS_EXT2FS_EXTERN_H_
#define _UFS_EXT2FS_EXT2FS_EXTERN_H_

struct buf;
struct fid;
struct m_ext2fs;
struct inode;
struct mount;
struct nameidata;
struct lwp;
struct proc;
struct statvfs;
struct timeval;
struct ufsmount;
struct uio;
struct vnode;
struct mbuf;
struct componentname;

extern struct pool ext2fs_inode_pool;		/* memory pool for inodes */
extern struct pool ext2fs_dinode_pool;		/* memory pool for dinodes */

#define	EXT2FS_ITIMES(ip, acc, mod, cre) \
	while ((ip)->i_flag & (IN_ACCESS | IN_CHANGE | IN_UPDATE | IN_MODIFY)) \
		ext2fs_itimes(ip, acc, mod, cre)

__BEGIN_DECLS

/* ext2fs_alloc.c */
int ext2fs_alloc(struct inode *, daddr_t, daddr_t , kauth_cred_t,
		   daddr_t *);
int ext2fs_realloccg(struct inode *, daddr_t, daddr_t, int, int ,
			  kauth_cred_t, struct buf **);
int ext2fs_valloc(struct vnode *, int, kauth_cred_t, struct vnode **);
/* XXX ondisk32 */
daddr_t ext2fs_blkpref(struct inode *, daddr_t, int, int32_t *);
void ext2fs_blkfree(struct inode *, daddr_t);
int ext2fs_vfree(struct vnode *, ino_t, int);

/* ext2fs_balloc.c */
int ext2fs_balloc(struct inode *, daddr_t, int, kauth_cred_t,
			struct buf **, int);
int ext2fs_gop_alloc(struct vnode *, off_t, off_t, int, kauth_cred_t);

/* ext2fs_bmap.c */
int ext2fs_bmap(void *);

/* ext2fs_inode.c */
u_int64_t ext2fs_size(struct inode *);
int ext2fs_setsize(struct inode *, u_int64_t);
int ext2fs_update(struct vnode *, const struct timespec *,
    const struct timespec *, int);
int ext2fs_truncate(struct vnode *, off_t, int, kauth_cred_t);
int ext2fs_inactive(void *);

/* ext2fs_lookup.c */
int ext2fs_readdir(void *);
int ext2fs_lookup(void *);
int ext2fs_direnter(struct inode *, struct vnode *,
			 struct componentname *);
int ext2fs_dirremove(struct vnode *, struct componentname *);
int ext2fs_dirrewrite(struct inode *, struct inode *,
			   struct componentname *);
int ext2fs_dirempty(struct inode *, ino_t, kauth_cred_t);
int ext2fs_checkpath(struct inode *, struct inode *, kauth_cred_t);

/* ext2fs_subr.c */
int ext2fs_blkatoff(struct vnode *, off_t, char **, struct buf **);
void ext2fs_fragacct(struct m_ext2fs *, int, int32_t[], int);
void ext2fs_itimes(struct inode *, const struct timespec *,
    const struct timespec *, const struct timespec *);

/* ext2fs_vfsops.c */
VFS_PROTOS(ext2fs);
int ext2fs_reload(struct mount *, kauth_cred_t);
int ext2fs_mountfs(struct vnode *, struct mount *);
int ext2fs_flushfiles(struct mount *, int);
int ext2fs_sbupdate(struct ufsmount *, int);
int ext2fs_cgupdate(struct ufsmount *, int);
void ext2fs_set_inode_guid(struct inode *);

/* ext2fs_readwrite.c */
int ext2fs_read(void *);
int ext2fs_write(void *);

/* ext2fs_vnops.c */
int ext2fs_create(void *);
int ext2fs_mknod(void *);
int ext2fs_open(void *);
int ext2fs_access(void *);
int ext2fs_getattr(void *);
int ext2fs_setattr(void *);
int ext2fs_remove(void *);
int ext2fs_link(void *);
int ext2fs_rename(void *);
int ext2fs_mkdir(void *);
int ext2fs_rmdir(void *);
int ext2fs_symlink(void *);
int ext2fs_readlink(void *);
int ext2fs_advlock(void *);
int ext2fs_fsync(void *);
int ext2fs_vinit(struct mount *, int (**specops)(void *),
		      int (**fifoops)(void *), struct vnode **);
int ext2fs_makeinode(int, struct vnode *, struct vnode **,
			  struct componentname *cnp);
int ext2fs_reclaim(void *);

__END_DECLS

#define IS_EXT2_VNODE(vp)   (vp->v_tag == VT_EXT2FS)

extern int (**ext2fs_vnodeop_p)(void *);
extern int (**ext2fs_specop_p)(void *);
extern int (**ext2fs_fifoop_p)(void *);

#endif /* !_UFS_EXT2FS_EXT2FS_EXTERN_H_ */
