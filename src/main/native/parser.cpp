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

#include "http_parser.h"
#include "net_java_httpparser_HttpParser.h"

#if _MSC_VER
#define snprintf _snprintf
#endif

static JNIEnv* _env = NULL;
static jclass _settings_cid = NULL;
static jmethodID _on_message_begin_mid = NULL;
static jmethodID _on_url_mid = NULL;
static jmethodID _on_header_field_mid = NULL;
static jmethodID _on_header_value_mid = NULL;
static jmethodID _on_headers_complete_mid = NULL;
static jmethodID _on_body_mid = NULL;
static jmethodID _on_message_complete_mid = NULL;

static int call_cb(jmethodID mid, http_parser* parser) {
  assert(_env);
  assert(parser);
  assert(parser->data);
  jobject settings = (jobject) parser->data;
  return _env->CallIntMethod(settings, mid);
}

static int call_data_cb(jmethodID mid, http_parser* parser, const char *at, size_t length) {
  assert(_env);
  assert(parser);
  assert(parser->data);
  jobject settings = (jobject) parser->data;
  jbyteArray bytes = _env->NewByteArray(length);
  _env->SetByteArrayRegion(bytes, 0, length, reinterpret_cast<const jbyte*>(at));
  return _env->CallIntMethod(settings, mid, bytes);
}

static int _on_message_begin_cb(http_parser* parser) {
  return call_cb(_on_message_begin_mid, parser);
}

static int _on_url_cb(http_parser* parser, const char *at, size_t length) {
  return call_data_cb(_on_url_mid, parser, at, length);
}

static int _on_header_field_cb(http_parser* parser, const char *at, size_t length) {
  return call_data_cb(_on_header_field_mid, parser, at, length);
}

static int _on_header_value_cb(http_parser* parser, const char *at, size_t length) {
  return call_data_cb(_on_header_value_mid, parser, at, length);
}

static int _on_headers_complete_cb(http_parser* parser) {
  return call_cb(_on_headers_complete_mid, parser);
}

static int _on_body_cb(http_parser* parser, const char *at, size_t length) {
  return call_data_cb(_on_body_mid, parser, at, length);
}

static int _on_message_complete_cb(http_parser* parser) {
  return call_cb(_on_message_complete_mid, parser);
}

/*
 * Class:     net_java_httpparser_HttpParser
 * Method:    _new
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_net_java_httpparser_HttpParser__1new
  (JNIEnv *env, jclass cls) {

  _settings_cid = env->FindClass("net/java/httpparser/HttpParserSettings");
  assert(_settings_cid);
  _settings_cid = (jclass) env->NewGlobalRef(_settings_cid);
  assert(_settings_cid);

  _on_message_begin_mid = env->GetMethodID(_settings_cid, "onMessageBegin", "()I");
  assert(_on_message_begin_mid);

  _on_url_mid = env->GetMethodID(_settings_cid, "onURL", "([B)I");
  assert(_on_url_mid);

  _on_header_field_mid = env->GetMethodID(_settings_cid, "onHeaderField", "([B)I");
  assert(_on_header_field_mid);

  _on_header_value_mid = env->GetMethodID(_settings_cid, "onHeaderValue", "([B)I");
  assert(_on_header_value_mid);

  _on_headers_complete_mid = env->GetMethodID(_settings_cid, "onHeadersComplete", "()I");
  assert(_on_headers_complete_mid);

  _on_body_mid = env->GetMethodID(_settings_cid, "onBody", "([B)I");
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
 * Signature: (JLnet/java/httpparser/HttpParserSettings;[BII)J
 */
JNIEXPORT jlong JNICALL Java_net_java_httpparser_HttpParser__1execute
  (JNIEnv *env, jobject that, jlong ptr, jobject settings, jbyteArray buffer, jint offset, jint length) {

  assert(ptr);
  http_parser* parser = reinterpret_cast<http_parser*>(ptr);
  parser->data = env->NewGlobalRef(settings);
  assert(parser->data);
  _env = env;

  http_parser_settings ps;
  ps.on_message_begin = _on_message_begin_cb;
  ps.on_url = _on_url_cb;
  ps.on_header_field = _on_header_field_cb;
  ps.on_header_value = _on_header_value_cb;
  ps.on_headers_complete = _on_headers_complete_cb;
  ps.on_body = _on_body_cb;
  ps.on_message_complete = _on_message_complete_cb;

  jbyte* data = new jbyte[length];
  env->GetByteArrayRegion(buffer, offset, length, data);
  size_t r = http_parser_execute(parser, &ps, reinterpret_cast<const char*>(data), length);
  delete[] data;
  env->DeleteGlobalRef((jobject) parser->data);
  parser->data = NULL;
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
