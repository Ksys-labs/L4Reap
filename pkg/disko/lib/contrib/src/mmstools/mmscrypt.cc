/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#ifdef __HAVE_MMSCRYPT__
#include <errno.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/aes.h>

#include "mmstools/mmscrypt.h"
#include "mmstools/mmsfile.h"

/**
 * Creates an SSL key that will be saved in the given file.
 *
 * @param	keyfile		save encrypted key to this file
 * @return  unencrypted key (NULL if error occured)
 *
 * @note	The memory for the returned key has to be freed.
 *
 * @see		MMSCrypt::getUserKey()
*/
unsigned char* MMSCrypt::createUserKey(string keyfile) {
    MMSFile        *file;
    unsigned char  *userKey, *userKeyEnc;
    size_t         numWritten = 0;

    /* generate random key */
    RAND_set_rand_method(RAND_SSLeay());
	userKey = (unsigned char*)malloc(EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH);
	if(!userKey) {
		return NULL;
	}
    RAND_bytes(userKey, EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH);
    RAND_cleanup();

    userKeyEnc = encrypt(userKey, EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH, true);

    /* write key to file */
    file = new MMSFile(keyfile, MMSFM_WRITE);
    file->writeBuffer((void*)userKeyEnc, &numWritten, 16, 1);
    delete(file);

    return userKey;
}

/**
 * Returns an SSL key that was stored in the given file.
 * If the file doesn't exist, a new key will be generated
 * and saved.
 *
 * @param	keyfile		read encrypted key from this file
 * @return  unencrypted key (NULL if error occured)
 *
 * @note	The memory for the returned key has to be freed.
 *
 * @see		MMSCrypt::createUserKey()
 *
 * @exception MMSCryptError File could not be opened.
*/
unsigned char* MMSCrypt::getUserKey(string keyfile) {
    unsigned char *userKey, *userKeyEnc;
    MMSFile       *file;
    size_t  numRead = 0;

    /* try to open keyfile for reading                   */
    /* if it fails and the filename differs from default */
    /* try the default file                              */
    file = new MMSFile(keyfile);
    if((file->getLastError() != 0) &&
       (keyfile != MMSCRYPT_DEFAULT_KEY_FILENAME))
       file = new MMSFile(MMSCRYPT_DEFAULT_KEY_FILENAME);

    switch(file->getLastError()) {
        case 0 :
            file->readBufferEx((void**)&userKeyEnc, &numRead);
            userKey = decrypt(userKeyEnc, EVP_MAX_KEY_LENGTH + EVP_MAX_IV_LENGTH, true);
            delete(file);
            break;
        case ENOENT :
            delete(file);
            userKey = createUserKey(file->getName());
            break;
        default :
            delete(file);
            throw MMSCryptError(0, "file " + keyfile + " could not be opened (" + strerror(file->getLastError()) + ")");
    }

    return userKey;
}

MMSCrypt::MMSCrypt(string keyfile) {
    unsigned char mmskey[] = {0x25, 0x04, 0x19, 0x79, 0xaf, 0xfe, 0x1a, 0x3d};
    unsigned char mmsiv[]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    unsigned char *userKey = NULL;

    /* initialise our private cipher context */
    EVP_CIPHER_CTX_init(&mmsCtx);
    EVP_EncryptInit_ex(&mmsCtx, EVP_aes_128_cbc(), 0, mmskey, mmsiv);

    /* initialise user cipher context */
    userKey = getUserKey(keyfile);
    EVP_CIPHER_CTX_init(&userCtx);
    EVP_EncryptInit_ex(&userCtx, EVP_aes_128_cbc(), 0, &userKey[0], &userKey[EVP_MAX_KEY_LENGTH - 1]);

    /* free memory */
    free(userKey);
}

MMSCrypt::~MMSCrypt() {
    EVP_CIPHER_CTX_cleanup(&mmsCtx);
    EVP_CIPHER_CTX_cleanup(&userCtx);
}

unsigned char* MMSCrypt::encrypt(unsigned char *in, unsigned int size, bool useMMSCtx) {
    unsigned char  *out;
    int            inl, tmp, ol = 0;
    EVP_CIPHER_CTX *ctx;

    ((size == 0) ? inl = strlen((char*)in) : inl = size);
    (useMMSCtx ? ctx = &mmsCtx : ctx = &userCtx);

    if(!(out = (unsigned char*)malloc(inl + EVP_CIPHER_CTX_block_size(ctx))))
        throw MMSCryptError(0, "not enough memory available");

    for(int i = 0; i < inl / 128; i++) {
        if(!EVP_EncryptUpdate(ctx, &out[ol], &tmp, &in[ol], 128))
            throw MMSCryptError(0, "error while encrypting data");
        ol += tmp;
    }

    if(inl % 128) {
        if(!EVP_EncryptUpdate(ctx, &out[ol], &tmp, &in[ol], inl % 128))
            throw MMSCryptError(0, "error while encrypting data");
        ol += tmp;
    }

    if(!EVP_EncryptFinal_ex(ctx, &out[ol], &tmp))
        throw MMSCryptError(0, "error while encrypting data");

    return out;
}

unsigned char* MMSCrypt::decrypt(unsigned char *in, unsigned int size, bool useMMSCtx) {
    unsigned char  *out;
    int            inl, ol;
    EVP_CIPHER_CTX *ctx;

    ((size == 0) ? inl = strlen((char*)in) : inl = size);
    (useMMSCtx ? ctx = &mmsCtx : ctx = &userCtx);

    if(!(out = (unsigned char*)malloc(inl + EVP_CIPHER_CTX_block_size(ctx) + 1)))
        throw MMSCryptError(0, "not enough memory available");

    EVP_DecryptUpdate(ctx, out, &ol, in, inl);

    /* nothing to decrypt */
    if(!ol) {
        free(out);
        return 0;
    }

    /* null-terminate output */
    out[ol] = 0;
    return out;
}

#endif /* __HAVE_MMSCRYPT__ */
