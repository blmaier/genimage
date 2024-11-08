project(
  'genimage',
  'c',
  meson_version: '>=0.46.0',
  version: '18',
)

cc = meson.get_compiler('c')

cconf = [
  '-D_GNU_SOURCE',
  '-DDSO_HIDDEN=1',
  '-DPACKAGE_VERSION="@0@"'.format(meson.project_version()),
]

if cc.has_header('linux/fs.h')
  cconf += ['-DHAVE_LINUX_FS_H=1']
  if cc.has_header('linux/fiemap.h')
    cconf += ['-DHAVE_FIEMAP=1']
  endif
endif

if cc.has_function('fallocate', args : '-D_GNU_SOURCE', prefix : '#include<fcntl.h>')
  cconf += ['-DHAVE_FALLOCATE=1']
endif

confuse_dep = dependency('libconfuse', version: '>= 2.8')

if cc.has_function('cfg_add_searchpath', dependencies : confuse_dep, prefix : '#include<confuse.h>')
  cconf += ['-DHAVE_SEARCHPATH=1']
endif

genimage_source = files(
  'genimage.c',
  'config.c',
  'util.c',
  'crc32.c',
  'image-android-sparse.c',
  'image-cpio.c',
  'image-cramfs.c',
  'image-ext2.c',
  'image-f2fs.c',
  'image-btrfs.c',
  'image-file.c',
  'image-fip.c',
  'image-fit.c',
  'image-flash.c',
  'image-hd.c',
  'image-iso.c',
  'image-jffs2.c',
  'image-qemu.c',
  'image-rauc.c',
  'image-squashfs.c',
  'image-tar.c',
  'image-ubi.c',
  'image-ubifs.c',
  'image-vfat.c',
)

genimage_exe = executable(
  'genimage',
  genimage_source,
  c_args: cconf,
  dependencies: confuse_dep,
)

tests = [
  'genimage',
  'ext',
  'filesystem',
  'flash',
  'hdimage',
  'misc',
]
foreach test : tests
  test_script = find_program('test/@0@.test'.format(test))
  test(test, test_script, depends : genimage_exe)
endforeach
