/*
* Copyright (c) 2017, Intel Corporation
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:

* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
* 3. Neither the name of the copyright holder nor the names of its contributors
*    may be used to endorse or promote products derived from this software
*    without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
* OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TestUtils/DigestUtils.h"

#include <OpensslHelpers/SignatureVerification.h>
#include <OpensslHelpers/KeyUtils.h>
#include <gtest/gtest.h>

#include "TestUtils/KeyHelpers.h"
#include "TestUtils/DigestUtils.h"

using namespace intel::sgx;

TEST(keyUtilsTest, rawTo256EcdsaKeyShouldReturnCorrectKey)
{
    // GIVEN
    auto prv = qvl::test::priv(qvl::test::PEM_PRV);
    auto evp = qvl::crypto::make_unique(EVP_PKEY_new());
    ASSERT_TRUE(1 == EVP_PKEY_set1_EC_KEY(evp.get(), prv.get()));
    auto pb = qvl::test::pub(qvl::test::PEM_PUB);
    ASSERT_TRUE(1 == EC_KEY_check_key(pb.get()));
    
    const std::vector<uint8_t> data(150, 0xff);
    const auto sig = qvl::DigestUtils::signMessageSha256(data, *evp);
    ASSERT_FALSE(sig.empty());
    ASSERT_TRUE(qvl::DigestUtils::verifySig(sig, data, *pb));

    // at this point we have valid priv and pub key
    // now when we convert pub key to bytes and covert
    // these bytes back to openssl struct we should be able
    // to successfully verify sig
    
    const auto rawUncompressedPubKeyWithoutHeader = qvl::test::getRawPub(*pb);
    ASSERT_TRUE(64 == rawUncompressedPubKeyWithoutHeader.size());
    std::array<uint8_t, 64> arr;
    std::copy_n(rawUncompressedPubKeyWithoutHeader.begin(), 64, arr.begin());
    
    // WHEN
    const auto newPbKey = qvl::crypto::rawToP256PubKey(arr);
    ASSERT_TRUE(nullptr != newPbKey);

    // THEN
    EXPECT_TRUE(qvl::DigestUtils::verifySig(sig, data, *newPbKey));
} 
