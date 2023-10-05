/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set et sw=2 ts=2: */
/***************************************************************************
 *            hugin.c
 *
 *  Thu Nov  1 13:38:47 CET 2012
 *  Copyright 2012 Bent Bisballe Nyeng
 *  deva@aasimon.org
 ****************************************************************************/

/*
 *  This file is part of Hugin.
 *
 *  Hugin is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Hugin is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Hugin; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#include "hugin.h"

#ifndef DISABLE_HUGIN

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOGPERM 0660

#ifdef WITH_HUG_MUTEX
# ifdef WIN32
#  include <windows.h>
typedef HANDLE mutex_t;
# else
#  include <pthread.h>
typedef pthread_mutex_t mutex_t;
# endif

void mutex_init(mutex_t *mutex)
{
#ifdef WIN32
  *mutex = CreateMutex(NULL,  // default security attributes
                       FALSE, // initially not owned
                       NULL); // unnamed mutex
#else
  pthread_mutex_init (mutex, NULL);
#endif
}

void mutex_close(mutex_t *mutex)
{
#ifdef WIN32
  CloseHandle(*mutex);
#else
  pthread_mutex_destroy(mutex);
#endif
}

void mutex_lock(mutex_t *mutex)
{
#ifdef WIN32
  WaitForSingleObject(*mutex, // handle to mutex
                      INFINITE);  // no time-out interval
#else
  pthread_mutex_lock(mutex);
#endif
}

void mutex_unlock(mutex_t *mutex)
{
#ifdef WIN32
  ReleaseMutex(*mutex);
#else
  pthread_mutex_unlock(mutex);
#endif
}

#ifdef WIN32
#define MUTEX_INIT_VALUE 0
#else
#define MUTEX_INIT_VALUE {}
#endif

#else/*WITH_HUG_MUTEX*/

#define MUTEX_INIT_VALUE 0
typedef int mutex_t;
void mutex_init(mutex_t *mutex) { (void)mutex; }
void mutex_close(mutex_t *mutex) { (void)mutex; }
void mutex_lock(mutex_t *mutex) { (void)mutex; }
void mutex_unlock(mutex_t *mutex) { (void)mutex; }

#endif/*WITH_HUG_MUTEX*/

mutex_t localtime_mutex;

#ifdef WITH_HUG_FILTER
#include "hugin_filter.h"
#endif

#ifdef WITH_HUG_SYSLOG
#include "hugin_syslog.h"
#endif

struct hug_config_t {
  unsigned int flags;
  int fd;
  int file_fd;
  int stdout_no_date;
  mutex_t mutex;
  char *filename;
#ifdef WITH_HUG_SYSLOG
  const char* syslog_host;
  int syslog_port;
#endif
} hug_config = {
  HUG_FLAG_DEFAULT, // flags
  -1, // fd
  -1, // file_fd
  0, //stdout_no_date
  //#ifdef WITH_HUG_MUTEX
  MUTEX_INIT_VALUE, // mutex;
  //#endif
  NULL, // filename
#ifdef WITH_HUG_SYSLOG
  NULL, // syslog_host;
  541, // syslog_port;
#endif
  /* // This doesn't work in C++
  .flags = HUG_FLAG_DEFAULT,
  .fd = -1,
  .file_fd = -1,
  .stdout_no_date = 0
  */
}; 

struct tm *hug_localtime(const time_t *timep, struct tm *result)
{
  struct tm *res = NULL;
  mutex_lock(&localtime_mutex);

	if(timep && result) {
    memcpy(result,localtime(timep),sizeof(*result)); 
    res = result;
  }

  mutex_unlock(&localtime_mutex);
  return res;
}


static void hug_mutex_init()
{
#ifdef WITH_HUG_MUTEX
  if(hug_config.flags & HUG_FLAG_USE_MUTEX) {
    mutex_init(&hug_config.mutex);
  }

  mutex_init(&localtime_mutex);
#endif
}

static void hug_mutex_lock()
{
#ifdef WITH_HUG_MUTEX
  if(hug_config.flags & HUG_FLAG_USE_MUTEX) {
    mutex_lock(&hug_config.mutex);
  }
#endif
}

static void hug_mutex_unlock()
{
#ifdef WITH_HUG_MUTEX
  if(hug_config.flags & HUG_FLAG_USE_MUTEX) {
    mutex_unlock(&hug_config.mutex);
  }
#endif
}

static void hug_mutex_close()
{
#ifdef WITH_HUG_MUTEX
  if(hug_config.flags & HUG_FLAG_USE_MUTEX) {
    // Make sure we don't destroy the mutex while another thread is using it.
    hug_mutex_lock();
    hug_mutex_unlock();
    mutex_close(&hug_config.mutex);
  }
  
  mutex_close(&localtime_mutex);
#endif
}

hug_status_t hug_init(unsigned int flags, ...)
{
  hug_status_t status = HUG_STATUS_OK;

  hug_config.flags = flags;

  int end = 0;

  va_list vl;
  va_start(vl, flags);
  while(!end) {
    int option = va_arg(vl, int);
    switch(option) {
    case HUG_OPTION_END:
      end = 1;
      break;
    case HUG_OPTION_FD:
      hug_config.fd = va_arg(vl, int);
      break;
    case HUG_OPTION_STDOUT_NO_DATE:
      hug_config.stdout_no_date = va_arg(vl, int);
      break;
    case HUG_OPTION_FILENAME:
    {
      const char *filename = (const char*) va_arg(vl, char*);
      if(!filename) break;

      if(hug_config.flags & HUG_FLAG_OUTPUT_TO_FILE) {
        hug_config.filename = strdup(filename);
        hug_config.file_fd = open(hug_config.filename,
                                  O_CREAT | O_APPEND | O_RDWR, LOGPERM);
        if(hug_config.file_fd == -1) {
          fprintf(stderr, "Could not open logfile for writing: %s\n",
                  hug_config.filename);
          return HUG_STATUS_ERROR;
        }
      }
    }
      break;
#ifdef WITH_HUG_SYSLOG
    case HUG_OPTION_SYSLOG_PORT:
      hug_config.syslog_port = va_arg(vl, int);
      break;
    case HUG_OPTION_SYSLOG_HOST:
      hug_config.syslog_host = (const char*)va_arg(vl, char*);
      break;
#endif
#ifdef WITH_HUG_FILTER
    case HUG_OPTION_FILTER:
      hug_filter_parse((const char*)va_arg(vl, char*));
      break;
#endif
    default:
      status = HUG_STATUS_UNKNOWN_OPTION;
      printf("option: %x\n", option);
      goto err;
    }
  }

  hug_mutex_init();

#ifdef WITH_HUG_SYSLOG
  if(hug_config.flags & HUG_FLAG_OUTPUT_TO_SYSLOG) {
    hug_syslog_init(hug_config.syslog_host, hug_config.syslog_port);
  }
#endif

 err:
  va_end(vl);

  return status;
}

void hug_close()
{
  if(hug_config.flags & HUG_FLAG_OUTPUT_TO_FILE) {
    if(hug_config.file_fd != -1) close(hug_config.file_fd);
    if(hug_config.filename) free(hug_config.filename);
  }

#ifdef WITH_HUG_SYSLOG
  hug_syslog_close();
#endif

  hug_mutex_close();
}

hug_status_t hugin_reopen_log()
{
  if((hug_config.flags & HUG_FLAG_OUTPUT_TO_FILE) == 0) return HUG_STATUS_OK;

  close(hug_config.file_fd);
  hug_config.file_fd = open(hug_config.filename,
                            O_CREAT | O_APPEND | O_RDWR, LOGPERM);

  if(hug_config.file_fd == -1) {
    fprintf(stderr, "Could not re-open logfile for writing: %s\n",
            hug_config.filename);
    return HUG_STATUS_ERROR;
  }

  return HUG_STATUS_OK;
}

/*
static unsigned int gettid()
{
  return (unsigned int)pthread_self();
}
*/

static int hug_create_header(char *hdr, size_t size)
{
  time_t rawtime = time(NULL);
  struct tm t;
  hug_localtime(&rawtime, &t);

  return snprintf(hdr, size,
                  "%d-%02d-%02d %02d:%02d:%02d",
                  t.tm_year + 1900,
                  t.tm_mon + 1,
                  t.tm_mday,
                  t.tm_hour,
                  t.tm_min,
                  t.tm_sec);
}

static int hug_output_fd(int fd, const char *msg, int withdate)
{
  int s;

  if(fd == -1) return 1;

  if(withdate) {
    char hdr[32];
    hug_create_header(hdr, sizeof(hdr));
    
    s = write(fd, hdr, strlen(hdr));
    s = write(fd, " ", 1);
  }

  s = write(fd, msg, strlen(msg));
  if(msg[strlen(msg) - 1] != '\n') {
    s = write(fd, "\n", 1);
  }

  (void)s;

  return 0;
}

// These functions returns how big a buffer is needed to "render" the arg list.
static int vscprintf(const char *fmt, va_list va)
{
#ifdef WIN32
  return _vscprintf(fmt, va);
#else
  return vsnprintf(NULL, 0, fmt, va);
#endif
}

static int scprintf(const char *fmt, ...)
{
  int size;
  va_list va;
  va_start(va, fmt);
  size = vscprintf(fmt, va);
  va_end(va);

  return size;
}

#define HDR_ARGS debug_class_str[(unsigned)cl], ch, func, line

int __hugin__debug(const char *func, const int line,
                   const enum __debug_class cl,
                   const char *ch, const char *fmt, ...)
{
  int result = 0;
  int sz;
  char *buf = NULL;
  int hdr_bufsz = 0;
  int msg_bufsz = 0;
  const char hdr_fmt[] = "%s:%s:%s:%d ";

  // NOTE: This must be identical to the debug_class_str in debug_filter.c
  const char * const debug_class_str[] =
    { "fixme", "err", "warn", "info", "debug" };

  hug_mutex_lock();

#ifdef WITH_HUG_FILTER
  if(!hug_filter_enabled(cl, ch)) goto done;
#endif

  //
  // Generate message
  //
  {
    va_list va;
    va_start(va, fmt);
    hdr_bufsz = scprintf(hdr_fmt, HDR_ARGS);
    msg_bufsz = vscprintf(fmt, va);
    if(hdr_bufsz < 0 || msg_bufsz < 0) return 1; // Bad format?
    va_end(va);
  }

  buf = (char*)malloc(hdr_bufsz + msg_bufsz + 1);
  if(!buf) return 1; // Out of memory

  sz = sprintf(buf, hdr_fmt, HDR_ARGS);
  if(sz < 0) return 1; // Unknown error

  {
    va_list va;
    va_start(va, fmt);
    sz = vsprintf(buf + sz, fmt, va);
    if(sz < 0) return 1; // Unknown error
    va_end(va);
  }

  //
  // Send message to output
  //

  if(hug_config.flags & HUG_FLAG_OUTPUT_TO_STDOUT) {
    hug_output_fd(STDOUT_FILENO, buf, hug_config.stdout_no_date == 0);
  }

  if(hug_config.flags & HUG_FLAG_OUTPUT_TO_STDERR) {
    hug_output_fd(STDERR_FILENO, buf, 1);
  }

  if(hug_config.flags & HUG_FLAG_OUTPUT_TO_FD) {
    hug_output_fd(hug_config.fd, buf, 1);
  }

  if(hug_config.flags & HUG_FLAG_OUTPUT_TO_FILE) {
    hug_output_fd(hug_config.file_fd, buf, 1);
  }

#ifdef WITH_HUG_SYSLOG
  if(hug_config.flags & HUG_FLAG_OUTPUT_TO_SYSLOG) {
    hug_syslog_output(buf);
  }
#endif

#ifdef WITH_HUG_FILTER
done:
#endif
  hug_mutex_unlock();

  if(buf) free(buf);

  return result;
}

#endif/*DISABLE_HUGIN*/
