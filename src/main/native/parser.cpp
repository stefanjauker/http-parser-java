/*
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.  Oracle designates this
 * particular file as subject to the "Classpath" exception as provided
 * by Oracle in the LICENSE file that accompanied this code.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#include <assert.h>
#include <string.h>

#include <string>
#include <vector>

#include "http_parser.h"
#include "net_java_httpparser_HttpParser.h"

#if _MSC_VER
#define snprintf _snprintf
#endif

using namespace std;

static JNIEnv* _env = NULL;
static jclass _settings_cid = NULL;
static jmethodID _on_headers_complete_mid = NULL;
static jmethodID _on_headers_mid = NULL;
static jmethodID _on_body_mid = NULL;
static jmethodID _on_message_complete_mid = NULL;
static jclass _string_cid = NULL;

class DataHolder {
 private:
   jobject _settings;
   const char* _data;
   JNIEnv* _env;

   string _url;
   vector<string> _headers;
   int _hkeys;
   int _hvalues;

   int send_headers(jmethodID mid);

 public:
   inline jobject settings() { return _settings; }
   inline const char* data() { return _data; }
   inline void set_data(const char* data) { _data = data; }
   inline string& url() { return _url; }
   inline int on_message_begin() { _url.clear(); return 0; }
   void push_header_key(const string& key);
   void push_header_value(const string& value);
   int on_headers_complete();
   void on_message_complete();
   DataHolder(JNIEnv* env, jobject settings, const char* data);
   ~DataHolder();
};

DataHolder::DataHolder(JNIEnv* env, jobject settings, const char* data) {
  assert(env);
  assert(settings);
  assert(data);
  _env = env;
  _settings = (jobject) _env->NewGlobalRef(settings);
  _data = data;
  _hkeys = 0;
  _hvalues = 0;
}

DataHolder::~DataHolder() {
 _env->DeleteGlobalRef(_settings);
}

void DataHolder::push_header_key(const string& key) {
  if (_hkeys == _hvalues) { // new field
    _hkeys++;
    _headers.push_back(key);
  } else { // truncated field
    _headers.at(_headers.size() - 1).append(key);
  }
}

void DataHolder::push_header_value(const string& value) {
  if (_hvalues != _hkeys) { // new value
    _hvalues++;
    _headers.push_back(value);
  } else { // truncated value
    _headers.at(_headers.size() - 1).append(value);
  }
}

void DataHolder::on_message_complete() {
  if (_hkeys != 0) {
      this->send_headers(_on_headers_mid);
  }
}

int DataHolder::on_headers_complete() {
  return this->send_headers(_on_headers_complete_mid);
}

int DataHolder::send_headers(jmethodID mid) {
  int size = static_cast<int>(_headers.size());
  jobjectArray headers = _env->NewObjectArray(size, _string_cid, 0);
  for (int i = 0; i < size - 1; i+=2) {
    jstring field = _env->NewStringUTF(_headers.at(i).data());
    jstring value = _env->NewStringUTF(_headers.at(i+1).data());
    _env->SetObjectArrayElement(headers, i, field);
    _env->SetObjectArrayElement(headers, i+1, value);
  }
  _hkeys = _hvalues = 0;
  _headers.clear();
  jstring url = _env->NewStringUTF(_url.data());
  return _env->CallIntMethod(_settings, mid, url, headers);
}

static int call_cb(jmethodID mid, http_parser* parser) {
  assert(_env);
  assert(parser);
  assert(parser->data);
  DataHolder* holder = (DataHolder*) parser->data;
  return _env->CallIntMethod(holder->settings(), mid);
}

static int call_data_cb(jmethodID mid, http_parser* parser, const char* at, size_t length) {
  assert(_env);
  assert(parser);
  assert(parser->data);
  DataHolder* holder = (DataHolder*) parser->data;
  const size_t offset = at - holder->data();
  return _env->CallIntMethod(holder->settings(), mid, offset, length);
}

static int _on_message_begin_cb(http_parser* parser) {
  assert(_env);
  assert(parser);
  assert(parser->data);
  DataHolder* holder = (DataHolder*) parser->data;
  return holder->on_message_begin();
}

static int _on_url_cb(http_parser* parser, const char* at, size_t length) {
  assert(parser->data);
  DataHolder* holder = (DataHolder*) parser->data;
  holder->url().append(string(at, length));
  return 0;
}

static int _on_header_field_cb(http_parser* parser, const char* at, size_t length) {
  assert(parser->data);
  DataHolder* holder = (DataHolder*) parser->data;
  holder->push_header_key(string(at, length));
  return 0;
}

static int _on_header_value_cb(http_parser* parser, const char* at, size_t length) {
  assert(parser->data);
  DataHolder* holder = (DataHolder*) parser->data;
  holder->push_header_value(string(at, length));
  return 0;
}

static int _on_headers_complete_cb(http_parser* parser) {
  assert(parser->data);
  DataHolder* holder = (DataHolder*) parser->data;
  return holder->on_headers_complete();
}

static int _on_body_cb(http_parser* parser, const char* at, size_t length) {
  return call_data_cb(_on_body_mid, parser, at, length);
}

static int _on_message_complete_cb(http_parser* parser) {
  assert(parser->data);
  DataHolder* holder = (DataHolder*) parser->data;
  holder->on_message_complete();
  return call_cb(_on_message_complete_mid, parser);
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _new
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_net_java_httpparser_HttpParser__1new
  (JNIEnv *env, jclass cls) {

  _string_cid = env->FindClass("java/lang/String");
  assert(_string_cid);
  _string_cid = (jclass) env->NewGlobalRef(_string_cid);

  _settings_cid = env->FindClass("net/java/httpparser/HttpParserSettings");
  assert(_settings_cid);
  _settings_cid = (jclass) env->NewGlobalRef(_settings_cid);
  assert(_settings_cid);

  _on_headers_complete_mid = env->GetMethodID(_settings_cid, "onHeadersComplete", "(Ljava/lang/String;[Ljava/lang/String;)I");
  assert(_on_headers_complete_mid);

  _on_headers_mid = env->GetMethodID(_settings_cid, "onHeaders", "(Ljava/lang/String;[Ljava/lang/String;)I");
  assert(_on_headers_mid);

  _on_body_mid = env->GetMethodID(_settings_cid, "onBody", "(II)I");
  assert(_on_body_mid);

  _on_message_complete_mid = env->GetMethodID(_settings_cid, "onMessageComplete", "()I");
  assert(_on_message_complete_mid);

  http_parser* parser = new http_parser();
  return reinterpret_cast<jlong>(parser);
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _init
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_net_java_httpparser_HttpParser__1init
  (JNIEnv *env, jobject that, jlong ptr, jint type) {

  assert(ptr);
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  http_parser_init(parser, (http_parser_type) type);
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _execute
 * Signature: (JLnet/java/httpparser/HttpParserSettings;Ljava/nio/ByteBuffer;[BII)J
 */
JNIEXPORT jlong JNICALL Java_net_java_httpparser_HttpParser__1execute
  (JNIEnv *env, jobject that, jlong ptr, jobject settings, jobject buffer, jbyteArray data, jint offset, jint length) {

  assert(ptr);
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  _env = env;

  http_parser_settings ps;
  ps.on_message_begin = _on_message_begin_cb;
  ps.on_url = _on_url_cb;
  ps.on_header_field = _on_header_field_cb;
  ps.on_header_value = _on_header_value_cb;
  ps.on_headers_complete = _on_headers_complete_cb;
  ps.on_body = _on_body_cb;
  ps.on_message_complete = _on_message_complete_cb;

  size_t r;
  if (data) {
    jbyte* base = new jbyte[length];
    env->GetByteArrayRegion(data, offset, length, base);
    const char* buff = reinterpret_cast<const char*>(base);
    if(parser->data != NULL) {
      DataHolder* holder =(DataHolder*) parser->data;
      holder->set_data(buff);
    } else {
      DataHolder* holder = new DataHolder(env, settings, buff);
      parser->data = holder;
    }
    r = http_parser_execute(parser, &ps, buff, length);
    delete[] base;
  } else {
    jbyte* base = (jbyte*) env->GetDirectBufferAddress(buffer);
    const char* buff = reinterpret_cast<const char*>(base + offset);
    if(parser->data != NULL) {
      DataHolder* holder =(DataHolder*) parser->data;
      holder->set_data(buff);
    } else {
      DataHolder* holder = new DataHolder(env, settings, buff);
      parser->data = holder;
    }
    r = http_parser_execute(parser, &ps, buff, length);
  }
  return r;
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _should_keep_alive
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_net_java_httpparser_HttpParser__1should_1keep_1alive
  (JNIEnv *env, jobject that, jlong ptr) {

  assert(ptr);
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  return http_should_keep_alive(parser) ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _upgrade
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_net_java_httpparser_HttpParser__1upgrade
  (JNIEnv *env, jobject that, jlong ptr) {

  assert(ptr);
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  return parser->upgrade ? JNI_TRUE : JNI_FALSE;
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _destroy
 * Signature: (J)
 */
JNIEXPORT void JNICALL Java_net_java_httpparser_HttpParser__1destroy
  (JNIEnv *env, jobject that, jlong ptr) {

  assert(ptr);
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  delete parser;
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _minor
 * Signature: (J)S
 */
JNIEXPORT jshort JNICALL Java_net_java_httpparser_HttpParser__1minor
  (JNIEnv *env, jobject that, jlong ptr) {
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  return parser->http_minor;
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _major
 * Signature: (J)S
 */
JNIEXPORT jshort JNICALL Java_net_java_httpparser_HttpParser__1major
  (JNIEnv *env, jobject that, jlong ptr) {
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  return parser->http_major;
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _method
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_java_httpparser_HttpParser__1method
  (JNIEnv *env, jobject that, jlong ptr) {
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  return env->NewStringUTF(http_method_str((http_method) parser->method));
}


/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _errno_name
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_java_httpparser_HttpParser__1errno_1name
  (JNIEnv *env, jobject that, jlong ptr) {
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  enum http_errno err = HTTP_PARSER_ERRNO(parser);
  return env->NewStringUTF(http_errno_name(err));
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _status_code
 * Signature: (J)S;
 */
JNIEXPORT jshort JNICALL Java_net_java_httpparser_HttpParser__1status_1code
  (JNIEnv *env, jobject that, jlong ptr) {
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  return parser->status_code;
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _version
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_net_java_httpparser_HttpParser__1version
  (JNIEnv *env, jobject that) {

  char version[32];
  memset(version, 0, sizeof(version));
  snprintf(version, sizeof(version), "%d.%d", HTTP_PARSER_VERSION_MAJOR, HTTP_PARSER_VERSION_MINOR);
  return env->NewStringUTF(version);
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _free
 * Signature: (J)
 */
JNIEXPORT void JNICALL Java_net_java_httpparser_HttpParser__1free
  (JNIEnv *env, jobject that, jlong ptr) {
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  if(parser->data != NULL) {
    DataHolder* holder = (DataHolder*)parser->data;
    parser->data = NULL;
    delete holder;
  }
}
