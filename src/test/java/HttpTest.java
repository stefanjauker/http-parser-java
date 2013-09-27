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

// Copyright Joyent, Inc. and other Node contributors.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

import net.java.httpparser.HttpParserSettings;
import net.java.httpparser.HttpParser;
import org.testng.Assert;
import org.testng.annotations.Test;

import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicBoolean;

public class HttpTest {

    @Test
    public void testFirefoxGet() throws UnsupportedEncodingException {
        final String raw =
            "GET /favicon.ico HTTP/1.1\r\n" +
            "Host: 0.0.0.0=5000\r\n" +
            "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n" +
            "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n" +
            "Accept-Language: en-us,en;q=0.5\r\n" +
            "Accept-Encoding: gzip,deflate\r\n" +
            "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n" +
            "Keep-Alive: 300\r\n" +
            "Connection: keep-alive\r\n" +
            "Content-Length: 9\r\n" +
            "\r\n" +
            "some body";
        final ByteBuffer buffer = ByteBuffer.wrap(raw.getBytes("utf-8"));

        final AtomicBoolean messageBeginCalled = new AtomicBoolean(false);
        final AtomicBoolean urlCalled = new AtomicBoolean(false);
        final AtomicBoolean headerFieldCalled = new AtomicBoolean(false);
        final AtomicBoolean headerValueCalled = new AtomicBoolean(false);
        final AtomicBoolean headersCompleteCalled = new AtomicBoolean(false);
        final AtomicBoolean bodyCalled = new AtomicBoolean(false);
        final AtomicBoolean messageCompleteCalled = new AtomicBoolean(false);

        final HttpParserSettings settings = new HttpParserSettings() {
            @Override
            public int onMessageBegin() {
                System.out.println("onMessageBegin");
                messageBeginCalled.set(true);
                return 0;
            }

            @Override
            public int onURL(byte[] data) {
                urlCalled.set(data != null && data.length > 0);
                final String url = new String(data);
                System.out.println("onURL " + url);
                Assert.assertEquals(url, "/favicon.ico");
                return 0;
            }

            @Override
            public int onHeaderField(byte[] data) {
                System.out.println("onHeaderField " + new String(data));
                headerFieldCalled.set(data != null && data.length > 0);
                return 0;
            }

            @Override
            public int onHeaderValue(byte[] data) {
                System.out.println("onHeaderValue " + new String(data));
                headerValueCalled.set(data != null && data.length > 0);
                return 0;
            }

            @Override
            public int onHeadersComplete() {
                System.out.println("onHeadersComplete");
                headersCompleteCalled.set(true);
                return 0;
            }

            @Override
            public int onBody(byte[] data) {
                bodyCalled.set(data != null && data.length > 0);
                final String body = new String(data);
                System.out.println("onBody " + body);
                Assert.assertEquals(body, "some body");
                return 0;
            }

            @Override
            public int onMessageComplete() {
                System.out.println("onMessageComplete");
                messageCompleteCalled.set(true);
                return 0;
            }
        };

        final HttpParser httpParser = new HttpParser();
        httpParser.init(HttpParser.Type.REQUEST);
        httpParser.execute(settings, buffer);

        Assert.assertTrue(messageBeginCalled.get());
        Assert.assertTrue(urlCalled.get());
        Assert.assertTrue(headerFieldCalled.get());
        Assert.assertTrue(headerValueCalled.get());
        Assert.assertTrue(headersCompleteCalled.get());
        Assert.assertTrue(bodyCalled.get());
        Assert.assertTrue(messageCompleteCalled.get());

        Assert.assertTrue(httpParser.shouldKeepAlive());

        httpParser.destroy();
    }

    @Test
    public void testPostIdentityBodyWorld() throws UnsupportedEncodingException {
        final String raw =
            "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n" +
            "Accept: */*\r\n" +
            "Transfer-Encoding: identity\r\n" +
            "Content-Length: 5\r\n" +
            "\r\n" +
            "World";
        final ByteBuffer buffer = ByteBuffer.wrap(raw.getBytes("utf-8"));

        final AtomicBoolean messageBeginCalled = new AtomicBoolean(false);
        final AtomicBoolean urlCalled = new AtomicBoolean(false);
        final AtomicBoolean headerFieldCalled = new AtomicBoolean(false);
        final AtomicBoolean headerValueCalled = new AtomicBoolean(false);
        final AtomicBoolean headersCompleteCalled = new AtomicBoolean(false);
        final AtomicBoolean bodyCalled = new AtomicBoolean(false);
        final AtomicBoolean messageCompleteCalled = new AtomicBoolean(false);

        final HttpParserSettings settings = new HttpParserSettings() {
            @Override
            public int onMessageBegin() {
                System.out.println("onMessageBegin");
                messageBeginCalled.set(true);
                return 0;
            }

            @Override
            public int onURL(byte[] data) {
                urlCalled.set(data != null && data.length > 0);
                final String url = new String(data);
                System.out.println("onURL " + url);
                Assert.assertEquals(url, "/post_identity_body_world?q=search#hey");
                return 0;
            }

            @Override
            public int onHeaderField(byte[] data) {
                System.out.println("onHeaderField " + new String(data));
                headerFieldCalled.set(data != null && data.length > 0);
                return 0;
            }

            @Override
            public int onHeaderValue(byte[] data) {
                System.out.println("onHeaderValue " + new String(data));
                headerValueCalled.set(data != null && data.length > 0);
                return 0;
            }

            @Override
            public int onHeadersComplete() {
                System.out.println("onHeadersComplete");
                headersCompleteCalled.set(true);
                return 0;
            }

            @Override
            public int onBody(byte[] data) {
                bodyCalled.set(data != null && data.length > 0);
                final String body = new String(data);
                System.out.println("onBody " + body);
                Assert.assertEquals(body, "World");
                return 0;
            }

            @Override
            public int onMessageComplete() {
                System.out.println("onMessageComplete");
                messageCompleteCalled.set(true);
                return 0;
            }
        };

        final HttpParser httpParser = new HttpParser();
        httpParser.init(HttpParser.Type.REQUEST);
        httpParser.execute(settings, buffer);

        Assert.assertTrue(messageBeginCalled.get());
        Assert.assertTrue(urlCalled.get());
        Assert.assertTrue(headerFieldCalled.get());
        Assert.assertTrue(headerValueCalled.get());
        Assert.assertTrue(headersCompleteCalled.get());
        Assert.assertTrue(bodyCalled.get());
        Assert.assertTrue(messageCompleteCalled.get());

        Assert.assertTrue(httpParser.shouldKeepAlive());

        httpParser.destroy();
    }

    @Test
    public void testGoogle301Response() throws UnsupportedEncodingException {
        final String raw =
            "HTTP/1.1 301 Moved Permanently\r\n" +
            "Location: http://www.google.com/\r\n" +
            "Content-Type: text/html; charset=UTF-8\r\n" +
            "Date: Sun, 26 Apr 2009 11:11:49 GMT\r\n" +
            "Expires: Tue, 26 May 2009 11:11:49 GMT\r\n" +
            "X-$PrototypeBI-Version: 1.6.0.3\r\n" + /* $ char in header field */
            "Cache-Control: public, max-age=2592000\r\n" +
            "Server: gws\r\n" +
            "Content-Length:  219  \r\n" +
            "\r\n" +
            "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n" +
            "<TITLE>301 Moved</TITLE></HEAD><BODY>\n" +
            "<H1>301 Moved</H1>\n" +
            "The document has moved\n" +
            "<A HREF=\"http://www.google.com/\">here</A>.\r\n" +
            "</BODY></HTML>\r\n";
        final ByteBuffer buffer = ByteBuffer.wrap(raw.getBytes("utf-8"));

        final AtomicBoolean messageBeginCalled = new AtomicBoolean(false);
        final AtomicBoolean urlCalled = new AtomicBoolean(false);
        final AtomicBoolean headerFieldCalled = new AtomicBoolean(false);
        final AtomicBoolean headerValueCalled = new AtomicBoolean(false);
        final AtomicBoolean headersCompleteCalled = new AtomicBoolean(false);
        final AtomicBoolean bodyCalled = new AtomicBoolean(false);
        final AtomicBoolean messageCompleteCalled = new AtomicBoolean(false);

        final HttpParserSettings settings = new HttpParserSettings() {
            @Override
            public int onMessageBegin() {
                System.out.println("onMessageBegin");
                messageBeginCalled.set(true);
                return 0;
            }

            @Override
            public int onURL(byte[] data) {
                urlCalled.set(data != null && data.length > 0);
                final String url = new String(data);
                System.out.println("onURL " + url);
                Assert.fail("unexpected url in response");
                return 0;
            }

            @Override
            public int onHeaderField(byte[] data) {
                System.out.println("onHeaderField " + new String(data));
                headerFieldCalled.set(data != null && data.length > 0);
                return 0;
            }

            @Override
            public int onHeaderValue(byte[] data) {
                System.out.println("onHeaderValue " + new String(data));
                headerValueCalled.set(data != null && data.length > 0);
                return 0;
            }

            @Override
            public int onHeadersComplete() {
                System.out.println("onHeadersComplete");
                headersCompleteCalled.set(true);
                return 0;
            }

            @Override
            public int onBody(byte[] data) {
                bodyCalled.set(data != null && data.length > 0);
                final String body = new String(data);
                System.out.println("onBody " + body);
                Assert.assertEquals(body,
                    "<HTML><HEAD><meta http-equiv=\"content-type\" content=\"text/html;charset=utf-8\">\n" +
                    "<TITLE>301 Moved</TITLE></HEAD><BODY>\n" +
                    "<H1>301 Moved</H1>\n" +
                    "The document has moved\n" +
                    "<A HREF=\"http://www.google.com/\">here</A>.\r\n" +
                    "</BODY></HTML>\r\n");
                return 0;
            }

            @Override
            public int onMessageComplete() {
                System.out.println("onMessageComplete");
                messageCompleteCalled.set(true);
                return 0;
            }
        };

        final HttpParser httpParser = new HttpParser();
        httpParser.init(HttpParser.Type.RESPONSE);
        httpParser.execute(settings, buffer);

        Assert.assertTrue(messageBeginCalled.get());
        Assert.assertFalse(urlCalled.get());
        Assert.assertTrue(headerFieldCalled.get());
        Assert.assertTrue(headerValueCalled.get());
        Assert.assertTrue(headersCompleteCalled.get());
        Assert.assertTrue(bodyCalled.get());
        Assert.assertTrue(messageCompleteCalled.get());

        Assert.assertTrue(httpParser.shouldKeepAlive());

        httpParser.destroy();
    }

}
