/*
 * NFS export support for the Linux hfsplus filesystem driver
 *
 * Copyright (c) 2014 Dwayne C. Litzenberger <dlitz@dlitz.net>
 *
 * This file is free software: you may copy, redistribute and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 2 of the
 * License, or (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <linux/exportfs.h>

#include "hfsplus_fs.h"

static struct inode *hfsplus_nfs_get_inode(struct super_block *sb,
		u64 ino, u32 generation)
{
	struct inode *inode;

	if (ino < HFSPLUS_FIRSTUSER_CNID && ino != HFSPLUS_ROOT_CNID)
		return ERR_PTR(-ESTALE);
	if (ino > (unsigned long)(~0ULL))
		return ERR_PTR(-ESTALE);

	inode = hfsplus_iget(sb, (unsigned long)ino);
	if (IS_ERR(inode))
		return ERR_CAST(inode);

	if (generation && inode->i_generation != generation) {
		iput(inode);
		return ERR_PTR(-ESTALE);
	}

	return inode;
}

static struct dentry *hfsplus_fh_to_dentry(struct super_block *sb,
		struct fid *fid, int fh_len, int fh_type)
{
	return generic_fh_to_dentry(sb, fid, fh_len, fh_type,
				    hfsplus_nfs_get_inode);
}

static struct dentry *hfsplus_fh_to_parent(struct super_block *sb,
		struct fid *fid, int fh_len, int fh_type)
{
	return generic_fh_to_parent(sb, fid, fh_len, fh_type,
				    hfsplus_nfs_get_inode);
}

const struct export_operations hfsplus_export_operations = {
	.fh_to_dentry = hfsplus_fh_to_dentry,
	.fh_to_parent = hfsplus_fh_to_parent,
	/* XXX - Documentation/filesystems/nfs/Exporting strongly
	 * recommends that we also implement .get_parent, but it seems to
	 * work fine without it (at least for Linux NFSv3 clients). */
};
