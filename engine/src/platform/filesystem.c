#include "filesystem.h"

#include "core/logger.h"
#include "core/lmemory.h"

#include <stdio.h>
#include <string.h>

#include <sys\stat.h>


b8 filesystem_exists(const char* path)
{
    struct stat buffer;
    return stat(path, &buffer) == 0;
}


b8 filesystem_open(const char* path, file_modes mode, b8 binary, file_handle* out_handle)
{
    out_handle->is_valid = false;
    out_handle->handle = 0;
    const char* mode_str;

    if((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) != 0) {
        mode_str = binary ? "w+b" : "w+";
    } else if ((mode & FILE_MODE_READ) != 0 && (mode & FILE_MODE_WRITE) == 0) {
        mode_str = binary ? "rb" : "r";
    } else if ((mode & FILE_MODE_READ) == 0 && (mode & FILE_MODE_WRITE) != 0) {
        mode_str = binary ? "wb" : "w";
    } else {
        LERROR("Invalid mode passed while trying to open file: '%s'", path);
        return false;
    }

    // Attempt to open file
    FILE* file = fopen(path, mode_str);
    if(!file) {
        LERROR("Error oppening file: '%s'", path);
        return false;
    }

    out_handle->handle = file;
    out_handle->is_valid = true;

    return true;
}

void filesystem_close(file_handle* handle)
{
    if(handle->handle) {
        fclose((FILE*)handle->handle);
        handle->handle = 0;
        handle->is_valid = false;
    }
}

b8 filesystem_read_line(file_handle* handle, char** line_buffer)
{
    if(!handle->handle) {
        return false;
    }
    // Since we're only reading a single line, we'll make an assumption that the line is less than 32K characters long
    char buffer[32000];
    if(fgets(buffer, 32000, (FILE*)handle->handle) != 0) {
        u64 length = strlen(buffer);
        *line_buffer = lallocate((sizeof(char) * length) + 1, MEMORY_TAG_STRING);
        strcpy(*line_buffer, buffer);
        return true;
    }

    return false;
}

b8 filesystem_write_line(file_handle* handle, const char* text)
{
    if(!handle->handle) {
        return false;
    }

    i32 result = fputs(text, (FILE*)handle->handle);
    if(result != EOF) {
        result = fputc('\n', (FILE*)handle->handle);
    }

    // Make sure to flush the stream so it is written to the file immediately.
    // This prevents data loss in the event of a crash.
    fflush((FILE*)handle->handle);
    return result != EOF;
}

b8 filesystem_read(file_handle* handle, u64 data_size, void* out_data, u64* out_bytes_read)
{
    if(!handle->handle || !out_data) {
        return false;
    }

    *out_bytes_read = fread(out_data, 1, data_size, (FILE*)handle->handle);
    
    if(*out_bytes_read != data_size) {
        return false;
    }

    return true;
}

b8 filesystem_read_all_bytes(file_handle* handle, u8** out_bytes, u64* out_bytes_read)
{
    if(!handle->handle) {
        return false;
    }

    // Get the size of the file
    fseek((FILE*)handle->handle, 0, SEEK_END);
    u64 size = ftell((FILE*)handle->handle);
    rewind((FILE*)handle->handle);

    *out_bytes = lallocate(sizeof(u8) * size, MEMORY_TAG_STRING);
    *out_bytes_read = fread(*out_bytes, 1, size, (FILE*)handle->handle);

    if(*out_bytes_read != size) {
        return false;
    }

    return true;
}

b8 filesystem_write(file_handle* handle, u64 data_size, const void* data, u64* out_bytes_written)
{
    if(!handle->handle) {
        return false;
    }

    *out_bytes_written = fwrite(data, 1, data_size, (FILE*)handle->handle);

    if(*out_bytes_written != data_size) {
        return false;
    }

    fflush((FILE*) handle->handle);

    return true;

}
