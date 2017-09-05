/** @file
  Test file for basic_string_view class
  @section license License
  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#include "catch.hpp"
#include "records/RecProtoTags.h"
#include "ink_memory_stubs.h"

#ifndef _DEBUG
#define _DEBUG
#endif

#include <string>
#include "ts/MemView.h"
#include "records/RecProtoTags.h"

const ts::StringView IP_PROTO_TAG_IPV4("ipv4", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_IPV6("ipv6", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_UDP("udp", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_TCP("tcp", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_TLS_1_0("tls/1.0", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_TLS_1_1("tls/1.1", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_TLS_1_2("tls/1.2", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_TLS_1_3("tls/1.3", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_HTTP_0_9("http/0.9", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_HTTP_1_0("http/1.0", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_HTTP_1_1("http/1.1", ts::StringView::literal);
const ts::StringView IP_PROTO_TAG_HTTP_2_0("h2", ts::StringView::literal); // HTTP/2 over TLS


SCENARIO("RecNormalizeProtoTag returns static pointers to the matching string")
{
  GIVEN("well-known name indices have been initialized") {
    ts_session_protocol_well_known_name_tags_init();

    WHEN("stack-allocated pointer for ipv4 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_IPV4 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("ipv4").c_str()) == IP_PROTO_TAG_IPV4.ptr());
      }
    }
    WHEN("stack-allocated pointer for ipv6 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_IPV6 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("ipv6").c_str()) == IP_PROTO_TAG_IPV6.ptr());
      }
    }
    WHEN("stack-allocated pointer for udp is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_UDP is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("udp").c_str()) == IP_PROTO_TAG_UDP.ptr());
      }
    }
    WHEN("stack-allocated pointer for tcp is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_TCP is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("tcp").c_str()) == IP_PROTO_TAG_TCP.ptr());
      }
    }
    WHEN("stack-allocated pointer for tls/1.0 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_TLS_1_0 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("tls/1.0").c_str()) == IP_PROTO_TAG_TLS_1_0.ptr());
      }
    }
    WHEN("stack-allocated pointer for tls/1.1 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_TLS_1_1 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("tls/1.1").c_str()) == IP_PROTO_TAG_TLS_1_1.ptr());
      }
    }
    WHEN("stack-allocated pointer for tls/1.2 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_TLS_1_2 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("tls/1.2").c_str()) == IP_PROTO_TAG_TLS_1_2.ptr());
      }
    }
    WHEN("stack-allocated pointer for tls/1.3 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_TLS_1_3 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("tls/1.3").c_str()) == IP_PROTO_TAG_TLS_1_3.ptr());
      }
    }
    WHEN("stack-allocated pointer for http/1.0 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_HTTP_1_0 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("http/1.0").c_str()) == IP_PROTO_TAG_HTTP_1_0.ptr());
      }
    }
    WHEN("stack-allocated pointer for http/1.1 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_HTTP_1_1 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("http/1.1").c_str()) == IP_PROTO_TAG_HTTP_1_1.ptr());
      }
    }
    WHEN("stack-allocated pointer for h2 is normalized") {
      THEN("the static pointer for IP_PROTO_TAG_HTTP_2_0 is returned") {
        REQUIRE(RecNormalizeProtoTag(std::string("h2").c_str()) == IP_PROTO_TAG_HTTP_2_0.ptr());
      }
    }
    WHEN("stack-allocated pointer for a bogus string is normalized") {
      THEN("null is returned") {
        std::string arbitraryString = "a8e9b0d9-28ce-4b78-882f-5d813d882f4d";
        REQUIRE(RecNormalizeProtoTag(arbitraryString.c_str()) == nullptr);
      }
    }
  }
}
