/*
* Copyright(C) 2011-2012 Alibaba Group Holding Limited
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation.
*/


#ifndef EASY_IO_H_
#define EASY_IO_H_

#include <easy_define.h>

#include <unistd.h>
#include <pthread.h>
#include "easy_io_struct.h"
#include "easy_log.h"

/**
 * IO文件头
 */

EASY_CPP_START

// 接口函数
///////////////////////////////////////////////////////////////////////////////////////////////////
// easy_io_t
extern easy_io_t           *easy_eio_create(easy_io_t *eio, int io_thread_count);
extern int                  easy_eio_start(easy_io_t *eio);
extern int                  easy_eio_wait(easy_io_t *eio);
extern int                  easy_eio_stop(easy_io_t *eio);
extern void                 easy_eio_destroy(easy_io_t *eio);

extern void                 easy_eio_set_uthread_start(easy_io_t *eio, easy_io_uthread_start_pt *on_utstart, void *args);
extern struct ev_loop      *easy_eio_thread_loop(easy_io_t *eio, int index);
extern void                 easy_eio_stat_watcher_start(easy_io_t *eio, ev_timer *stat_watcher,
        double interval, easy_io_stat_t *iostat, easy_io_stat_process_pt *process);

///////////////////////////////////////////////////////////////////////////////////////////////////
// easy_connection_t
extern easy_listen_t       *easy_connection_add_listen(easy_io_t *eio, const char *host, int port, easy_io_handler_pt *handler);
extern easy_connection_t   *easy_connection_connect_thread(easy_io_t *eio, easy_addr_t addr,
        easy_io_handler_pt *handler, int conn_timeout, void *args, int autoconn);
extern int                  easy_connection_connect(easy_io_t *eio, easy_addr_t addr,
        easy_io_handler_pt *handler, int conn_timeout, void *args, int autoconn);
extern int                  easy_connection_disconnect(easy_io_t *eio, easy_addr_t addr);
extern int                  easy_connection_disconnect_thread(easy_io_t *eio, easy_addr_t addr);
extern int                  easy_connection_send_session(easy_connection_t *c, easy_session_t *s);
///////////////////////////////////////////////////////////////////////////////////////////////////
// easy_session
extern easy_session_t      *easy_session_create(int size);
extern void                 easy_session_destroy(void *s);
///////////////////////////////////////////////////////////////////////////////////////////////////
// easy_client uthread
extern int                  easy_client_uthread_wait_conn(easy_connection_t *c);
extern int                  easy_client_uthread_wait_session(easy_session_t *s);
extern void                 easy_client_uthread_set_handler(easy_io_handler_pt *handler);
///////////////////////////////////////////////////////////////////////////////////////////////////
// easy_client_wait
extern void                 easy_client_wait_init(easy_client_wait_t *w);
extern void                 easy_client_wait(easy_client_wait_t *w, int count);
extern void                 easy_client_wait_cleanup(easy_client_wait_t *w);
extern void                 easy_client_wait_wakeup(easy_client_wait_t *w);
extern int                  easy_client_wait_process(easy_request_t *r);
//extern int                  easy_client_wait_on_connect(easy_connection_t *c);
extern int                  easy_client_wait_batch_process(easy_message_t *m);
extern void                *easy_client_send(easy_io_t *eio, easy_addr_t addr, easy_session_t *s);
extern int                  easy_client_dispatch(easy_io_t *eio, easy_addr_t addr, easy_session_t *s);
///////////////////////////////////////////////////////////////////////////////////////////////////
// easy_request
extern int                  easy_request_do_reply(easy_request_t *r);
extern easy_thread_pool_t  *easy_thread_pool_create(easy_io_t *eio, int cnt, easy_request_process_pt *cb, void *args);
extern easy_thread_pool_t  *easy_thread_pool_create_ex(easy_io_t *eio, int cnt, easy_baseth_on_start_pt *start,
        easy_request_process_pt *cb, void *args);
extern int                  easy_thread_pool_push(easy_thread_pool_t *tp, easy_request_t *r, uint64_t hv);
extern int                  easy_thread_pool_push_message(easy_thread_pool_t *tp, easy_message_t *m, uint64_t hv);
extern int                  easy_thread_pool_push_session(easy_thread_pool_t *tp, easy_session_t *s, uint64_t hv);
extern void                 easy_request_addbuf(easy_request_t *r, easy_buf_t *b);
extern void                 easy_request_addbuf_list(easy_request_t *r, easy_list_t *list);
extern void                 easy_request_wakeup(easy_request_t *r);
///////////////////////////////////////////////////////////////////////////////////////////////////
// easy_file
extern easy_file_task_t    *easy_file_task_create(easy_request_t *r, int fd, int bufsize);
extern void                 easy_file_task_reset(easy_file_task_t *ft, int type);

///////////////////////////////////////////////////////////////////////////////////////////////////
// define
#define EASY_IOTH_SELF ((easy_io_thread_t*) easy_baseth_self)
#define easy_session_set_args(s, a)     (s)->r.args = (void*)a
#define easy_session_set_timeout(s, t)  (s)->timeout = t
#define easy_request_set_wobj(r, w)     (r)->request_list_node.prev = (easy_list_t *)w
#define easy_session_set_wobj(s, w)     easy_request_set_wobj(&((s)->r), w)
#define easy_session_set_request(s, p, t, a)                \
    (s)->r.opacket = (void*) p;                             \
    (s)->r.args = (void*)a; (s)->timeout = t;
#define easy_session_packet_create(type, s, size)           \
    ((s = easy_session_create(size + sizeof(type))) ? ({    \
        memset(&((s)->data[0]), 0, sizeof(type));           \
        (s)->r.opacket = &((s)->data[0]);                   \
        (type*) &((s)->data[0]);}) : NULL)
#define easy_session_class_create(type, s, ...)             \
    ((s = easy_session_create(sizeof(type))) ? ({           \
        new(&((s)->data[0]))type(__VA_ARGS__);              \
        (s)->r.opacket = &((s)->data[0]);                   \
        (type*) &((s)->data[0]);}) : NULL)

//多eio管理器版
#ifdef EASY_MULTIPLICITY
#define easy_io_create(eio, cnt)                    easy_eio_create(eio, cnt)
#define easy_io_start(eio)                          easy_eio_start(eio)
#define easy_io_wait(eio)                           easy_eio_wait(eio)
#define easy_io_stop(eio)                           easy_eio_stop(eio)
#define easy_io_destroy(eio)                        easy_eio_destroy(eio)
#define easy_io_thread_loop(a,b)                    easy_eio_thread_loop(a,b)
#define easy_io_set_uthread_start(eio,start,args)   easy_eio_set_uthread_start(eio,start,args)
#define easy_io_stat_watcher_start(a1,a2,a3,a4,a5)  easy_eio_stat_watcher_start(a1,a2,a3,a4,a5)
#define easy_io_add_listen(eio,host,port,handler)   easy_connection_add_listen(eio,host,port,handler)
#define easy_io_connect(eio,addr,handler,t,args)    easy_connection_connect(eio,addr,handler,t,args,0)
#define easy_io_connect_thread(eio,addr,h,t,args)   easy_connection_connect_thread(eio,addr,h,t,args,0)
#define easy_io_disconnect(eio,addr)                easy_connection_disconnect(eio,addr)
#define easy_io_disconnect_thread(eio,addr)         easy_connection_disconnect_thread(eio,addr)
#define easy_request_thread_create(eio,cnt,cb,args) easy_thread_pool_create(eio,cnt,cb,args)
#define easy_io_dispatch(eio,addr,s)                easy_client_dispatch(eio,addr,s)
#define easy_io_send(eio,addr,s)                    easy_client_send(eio,addr,s);
#else
//单例版定义
#define easy_io_create(cnt)                         easy_eio_create(&easy_io_var, cnt)
#define easy_io_start()                             easy_eio_start(&easy_io_var)
#define easy_io_wait()                              easy_eio_wait(&easy_io_var)
#define easy_io_stop()                              easy_eio_stop(&easy_io_var)
#define easy_io_destroy()                           easy_eio_destroy(&easy_io_var)
#define easy_io_thread_loop(a)                      easy_eio_thread_loop(&easy_io_var,a)
#define easy_io_set_uthread_start(start,args)       easy_eio_set_uthread_start(&easy_io_var,start,args)
#define easy_io_stat_watcher_start(a1,a2,a3,a4)     easy_eio_stat_watcher_start(&easy_io_var,a1,a2,a3,a4)
#define easy_io_add_listen(host,port,handler)       easy_connection_add_listen(&easy_io_var,host,port,handler)
#define easy_io_connect(addr,handler,t,args)        easy_connection_connect(&easy_io_var,addr,handler,t,args,0)
#define easy_io_connect_thread(addr,h,t,args)       easy_connection_connect_thread(&easy_io_var,addr,h,t,args,0)
#define easy_io_disconnect(addr)                    easy_connection_disconnect(&easy_io_var,addr)
#define easy_io_disconnect_thread(addr)             easy_connection_disconnect_thread(&easy_io_var,addr)
#define easy_request_thread_create(cnt, cb, args)   easy_thread_pool_create(&easy_io_var, cnt, cb, args)
#define easy_io_dispatch(addr,s)                    easy_client_dispatch(&easy_io_var,addr,s)
#define easy_io_send(addr,s)                        easy_client_send(&easy_io_var,addr,s);
#endif

// 变量
extern __thread easy_baseth_t *easy_baseth_self;
extern easy_io_t easy_io_var;

EASY_CPP_END

#endif
