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

package net.java.httpparser;

import java.nio.ByteBuffer;

public final class HttpParser {

    public enum Type {
        // must be equal to enum http_parser_type values in http_parser.h
        REQUEST(0),
        RESPONSE(1),
        BOTH(2);

        final int value;

        Type(final int value) {
            this.value = value;
        }
    }

    private final long pointer;
    private boolean destroyed;

    static {
        System.loadLibrary("avatar-js");
    }

    public HttpParser() {
        pointer = _new();
        destroyed = false;
    }

    public void init(final Type type) {
        _init(pointer, type.value);
    }

    public short minor() {
        return _minor(pointer);
    }

    public short major() {
        return _major(pointer);
    }

    public String method() {
        return _method(pointer);
    }

    public String version() {
        return _version();
    }

    public String errnoName() {
        return _errno_name(pointer);
    }

    public short statusCode() {
        return _status_code(pointer);
    }

    public long execute(final HttpParserSettings settings, final ByteBuffer buffer) {
        return buffer.hasArray() ?
                _execute(pointer, settings, buffer, buffer.array(), buffer.position(), buffer.limit()) :
                _execute(pointer, settings, buffer, null, buffer.position(), buffer.limit());
    }

    public boolean shouldKeepAlive() {
        return _should_keep_alive(pointer);
    }

    public boolean upgrade() {
        return _upgrade(pointer);
    }

    public void destroy() {
        if (!destroyed) {
            _destroy(pointer);
            destroyed = true;
        }
    }

    @Override
    protected void finalize() throws Throwable {
        destroy();
        super.finalize();
    }

    private static native long _new();

    private native void _init(final long pointer, final int type);

    private native long _execute(final long pointer,
                                 final HttpParserSettings settings,
                                 final ByteBuffer buffer,
                                 final byte[] data,
                                 final int offset,
                                 final int length);

    private native String _version();

    private native boolean _should_keep_alive(final long pointer);

    private native boolean _upgrade(final long pointer);

    private native void _destroy(final long pointer);

    private native short _minor(final long pointer);

    private native short _major(final long pointer);

    private native String _method(final long pointer);

    private native String _errno_name(final long pointer);

    private native short _status_code(final long pointer);
}
