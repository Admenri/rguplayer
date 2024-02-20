// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD - style license that can be
// found in the LICENSE file.

#include "components/filesystem/filesystem.h"

#include "base/exceptions/exception.h"
#include "physfs.h"

namespace filesystem {

const Uint32 SDL_RWOPS_PHYSFS = SDL_RWOPS_UNKNOWN + 16;

namespace {

void ToLower(std::string& str) {
  for (size_t i = 0; i < str.size(); ++i)
    str[i] = tolower(str[i]);
}

const char* FindFileExtName(const char* filename) {
  for (size_t i = strlen(filename); i > 0; --i) {
    if (filename[i] == '/')
      break;
    if (filename[i] == '.')
      return filename + i + 1;
  }

  return nullptr;
}

inline PHYSFS_File* PHYSPtr(SDL_RWops* ops) {
  return static_cast<PHYSFS_File*>(ops->hidden.unknown.data1);
}

Sint64 PHYS_RWopsSize(SDL_RWops* ops) {
  PHYSFS_File* f = PHYSPtr(ops);
  if (!f)
    return -1;

  return PHYSFS_fileLength(f);
}

Sint64 PHYS_RWopsSeek(SDL_RWops* ops, int64_t offset, int whence) {
  PHYSFS_File* f = PHYSPtr(ops);
  if (!f)
    return -1;

  int64_t base;

  switch (whence) {
    default:
    case SDL_RW_SEEK_SET:
      base = 0;
      break;
    case SDL_RW_SEEK_CUR:
      base = PHYSFS_tell(f);
      break;
    case SDL_RW_SEEK_END:
      base = PHYSFS_fileLength(f);
      break;
  }

  int result = PHYSFS_seek(f, base + offset);
  return (result != 0) ? PHYSFS_tell(f) : -1;
}

size_t PHYS_RWopsRead(SDL_RWops* ops, void* buffer, size_t size) {
  PHYSFS_File* f = PHYSPtr(ops);
  if (!f)
    return 0;

  PHYSFS_sint64 result = PHYSFS_readBytes(f, buffer, size);
  return (result != -1) ? result : 0;
}

size_t PHYS_RWopsWrite(SDL_RWops* ops, const void* buffer, size_t size) {
  PHYSFS_File* f = PHYSPtr(ops);
  if (!f)
    return 0;

  PHYSFS_sint64 result = PHYSFS_writeBytes(f, buffer, size);

  return (result != -1) ? result : 0;
}

int PHYS_RWopsClose(SDL_RWops* ops) {
  PHYSFS_File* f = PHYSPtr(ops);
  if (!f)
    return -1;

  int result = PHYSFS_close(f);
  ops->hidden.unknown.data1 = 0;

  return (result != 0) ? 0 : -1;
}

int PHYS_RWopsCloseFree(SDL_RWops* ops) {
  int result = PHYS_RWopsClose(ops);
  SDL_DestroyRW(ops);

  return result;
}

void WrapperRWops(PHYSFS_File* handle, SDL_RWops& ops, bool auto_free) {
  ops.type = SDL_RWOPS_PHYSFS;
  ops.size = PHYS_RWopsSize;
  ops.seek = PHYS_RWopsSeek;
  ops.read = PHYS_RWopsRead;
  ops.write = PHYS_RWopsWrite;
  ops.close = PHYS_RWopsClose;
  if (auto_free)
    ops.close = PHYS_RWopsCloseFree;

  ops.hidden.unknown.data1 = handle;
}

struct OpenReadEnumData {
  Filesystem::OpenCallback callback;
  std::string full;
  std::string dir;
  std::string file;
  std::string ext;

  SDL_RWops* ops = nullptr;

  bool search_complete = false;
  int match_count = 0;

  std::string physfs_error;

  OpenReadEnumData() = default;
};

PHYSFS_EnumerateCallbackResult OpenReadEnumCallback(void* data,
                                                    const char* origdir,
                                                    const char* fname) {
  OpenReadEnumData* enum_data = static_cast<OpenReadEnumData*>(data);
  std::string filename(fname);
  ToLower(filename);

  if (enum_data->search_complete)
    return PHYSFS_ENUM_STOP;

  size_t it = filename.rfind(enum_data->file);
  if (it == std::string::npos)
    return PHYSFS_ENUM_OK;

  const char last = filename[enum_data->file.size()];
  if (last != '.' && last != '/')
    return PHYSFS_ENUM_OK;

  std::string fullpath;
  if (*origdir) {
    fullpath += std::string(origdir);
    fullpath += "/";
  }
  fullpath += filename;

  PHYSFS_File* file = PHYSFS_openRead(fullpath.c_str());
  if (!file) {
    enum_data->search_complete = true;
    enum_data->physfs_error = PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode());

    return PHYSFS_ENUM_ERROR;
  }

  WrapperRWops(file, *enum_data->ops, false);
  if (enum_data->callback.Run(enum_data->ops,
                              FindFileExtName(filename.c_str())))
    enum_data->search_complete = true;

  enum_data->match_count++;
  return PHYSFS_ENUM_OK;
}

}  // namespace

Filesystem::Filesystem(const std::string& argv0) {
  if (!PHYSFS_init(argv0.data())) {
    LOG(INFO) << "[Filesystem] Failed to load Physfs.";
  }
}

Filesystem::~Filesystem() {
  if (!PHYSFS_deinit())
    LOG(INFO) << "[Filesystem] Failed to unload Physfs.";
}

void Filesystem::AddLoadPath(const std::string& path) {
  if (!PHYSFS_mount(path.c_str(), nullptr, 1))
    LOG(INFO) << "[Filesystem] Failed to add path: " << path;
}

bool Filesystem::Exists(const std::string& filename) {
  return PHYSFS_exists(filename.c_str());
}

void Filesystem::OpenRead(const std::string& file_path, OpenCallback callback) {
  std::string filename(file_path);
  ToLower(filename);
  std::string dir, file, ext;

  size_t last_slash_pos = filename.find_last_of('/');
  if (last_slash_pos != std::string::npos) {
    dir = filename.substr(0, last_slash_pos);
    file = filename.substr(last_slash_pos + 1);
  } else
    file = filename;

  size_t last_dot_pos = file.find_last_of('.');
  if (last_dot_pos != std::string::npos) {
    ext = file.substr(last_dot_pos + 1);
    file = file.substr(0, last_dot_pos);
  }

  OpenReadEnumData data;
  data.callback = callback;

  data.full = filename;
  data.dir = dir;
  data.file = file;
  data.ext = ext;
  data.ops = SDL_CreateRW();

  PHYSFS_enumerate(dir.c_str(), OpenReadEnumCallback, &data);
  SDL_DestroyRW(data.ops);

  if (!data.physfs_error.empty())
    throw base::Exception::Exception(base::Exception::FilesystemError,
                                     "PhysFS: %s", data.physfs_error.c_str());

  if (data.match_count <= 0)
    throw base::Exception::Exception(base::Exception::NoFileError,
                                     "No file match: %s", filename.c_str());
}

void Filesystem::OpenReadRaw(const std::string& filename,
                             SDL_RWops& ops,
                             bool free_on_close) {
  PHYSFS_File* file = PHYSFS_openRead(filename.c_str());
  if (!file)
    throw base::Exception::Exception(base::Exception::NoFileError,
                                     "Failed to load file: %s",
                                     filename.c_str());

  WrapperRWops(file, ops, free_on_close);
}

}  // namespace filesystem
