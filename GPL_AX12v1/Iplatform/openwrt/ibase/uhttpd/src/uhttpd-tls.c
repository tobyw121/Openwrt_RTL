/*
 * uhttpd - Tiny single-threaded httpd - TLS helper
 *
 *   Copyright (C) 2010 Jo-Philipp Wich <xm@subsignal.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "uhttpd.h"
#include "uhttpd-tls.h"
#include "uhttpd-utils.h"

#include <syslog.h>
#define dbg(...) syslog(LOG_INFO, __VA_ARGS__)

SSL_CTX * uh_tls_ctx_init(void)
{
	SSL_CTX *c;

	SSL_load_error_strings();
	SSL_library_init();

#if TLS_IS_OPENSSL
	if ((c = SSL_CTX_new(SSLv23_server_method())) != NULL)
#else
	if ((c = SSL_CTX_new(TLSv1_server_method())) != NULL)
#endif
	{		
		SSL_CTX_set_verify(c, SSL_VERIFY_NONE, NULL);
#ifdef UH_SSL_CIPHERS_SERVER_PREFERENCE
		SSL_CTX_set_options(c, SSL_OP_CIPHER_SERVER_PREFERENCE);
#endif
	}

	return c;
}

int uh_tls_ctx_cert(SSL_CTX *c, const char *file)
{
	int rv;

	if( (rv = SSL_CTX_use_certificate_file(c, file, SSL_FILETYPE_PEM)) < 1 )
		rv = SSL_CTX_use_certificate_file(c, file, SSL_FILETYPE_ASN1);

	return rv;
}

int uh_tls_ctx_key(SSL_CTX *c, const char *file)
{
	int rv;

	if( (rv = SSL_CTX_use_PrivateKey_file(c, file, SSL_FILETYPE_PEM)) < 1 )
		rv = SSL_CTX_use_PrivateKey_file(c, file, SSL_FILETYPE_ASN1);

	return rv;
}

int uh_tls_ecdh_curve(SSL_CTX *c, const char *name)
{
#if OPENSSL_VERSION_NUMBER >= 0x0090800fL
#ifndef OPENSSL_NO_ECDH
    int      nid = 0;
    EC_KEY  *ecdh = NULL;

    /*
     * Elliptic-Curve Diffie-Hellman parameters are either "named curves"
     * from RFC 4492 section 5.1.1, or explicitly described curves over
     * binary fields. OpenSSL only supports the "named curves", which provide
     * maximum interoperability.
     */

    nid = OBJ_sn2nid(name);
    if (nid == 0)
	{
        HTTPS_D("Unknown curve name \"%s\"", name);
        return 0;
    }

    ecdh = EC_KEY_new_by_curve_name(nid);
    if (ecdh == NULL)
	{
        HTTPS_D("Unable to create curve \"%s\"", name);
        return 0;
    }

    SSL_CTX_set_options(c, SSL_OP_SINGLE_ECDH_USE);

    SSL_CTX_set_tmp_ecdh(c, ecdh);

    EC_KEY_free(ecdh);
#endif
#endif

    return 1;
}

int uh_tls_ctx_ciphers(SSL_CTX *c, const char *ciphers)
{
	int rv = 0;

	HTTPS_D("ciphers: %s \n", ciphers);
	if ((rv = SSL_CTX_set_cipher_list(c, ciphers)) < 1)
	{
 	    HTTPS_D("SSL_CTX_set_cipher_list(\"%s\") failed, ret = %d, use default\n", ciphers, rv);
		rv = SSL_CTX_set_cipher_list(c, UH_SSL_DEFAULT_CIPHERS);
	}
	
	if(rv > 0)
	{
		uh_tls_ecdh_curve(c, UH_SSL_DEFAULT_ECDH_CURVE);
	}

	return rv;
}

void uh_tls_ctx_free(struct listener *l)
{
	SSL_CTX_free(l->tls);
}

#if defined(TLS_ACCEPT_ASYNC)
void uh_tls_client_timeout(struct client *c)
{
	if(c->tls)
	{
		SSL_free(c->tls);
		c->tls = NULL;
	}
}

int uh_tls_client_accept(struct client *c)
{
	int rv, err;
	int fd = c->fd.fd;
	
	HTTPS_ASYNC("TLS: Client(%d) in accept\n", fd);
	
	if (!c->server || !c->server->tls)
	{
		HTTPS_ASYNC("TLS: HTTP accepted(%d)\n", fd);
		c->tls = NULL;
		return TLS_ACCEPTED;
	}

	if (!c->tls && (c->tls = SSL_new(c->server->tls)))
	{
		if ((rv = SSL_set_fd(c->tls, fd)) < 1)
		{		
			HTTPS_ASYNC("TLS: set fd(%d) = %p failed\n", fd, c->tls);
			SSL_free(c->tls);
			c->tls = NULL;
		}
		else
		{
			SSL_set_accept_state(c->tls);
		}
	}

	if(c->tls)
	{
		rv = SSL_do_handshake(c->tls);
		err = SSL_get_error(c->tls, rv);

		if ((rv != 1) &&
			(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE))
		{
			HTTPS_ASYNC("TLS: accepting(%d) = %p\n", fd, c->tls);
			return err == SSL_ERROR_WANT_READ ? TLS_WANT_READ : TLS_WANT_WRITE;
		}
		else if (rv == 1)
		{
			HTTPS_ASYNC("TLS: accepted(%d) = %p\n", fd, c->tls);
			return TLS_ACCEPTED;
		}

#ifdef TLS_IS_OPENSSL
		HTTPS_ASYNC("TLS: accept(%d) = failed: %s\n",
				fd, ERR_error_string(ERR_get_error(), NULL));
#endif
			
		SSL_free(c->tls);
		c->tls = NULL;
	}	

	return TLS_FAILED;
}
#else
int uh_tls_client_accept(struct client *c)
{
	int rv, err;
	int fd = c->fd.fd;

	if (!c->server || !c->server->tls)
	{
		c->tls = NULL;
		return 1;
	}

	if ((c->tls = SSL_new(c->server->tls)))
	{
		if ((rv = SSL_set_fd(c->tls, fd)) < 1)
		{
			SSL_free(c->tls);
			c->tls = NULL;
		}
		else
		{
			while (true)
			{
				rv = SSL_accept(c->tls);
				err = SSL_get_error(c->tls, rv);

				if ((rv != 1) &&
					(err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE))
				{
					if (uh_socket_wait(fd, c->server->conf->network_timeout,
									   (err == SSL_ERROR_WANT_WRITE)))
					{
						D("TLS: accept(%d) = retry\n", fd);
						continue;
					}

					D("TLS: accept(%d) = timeout\n", fd);
				}
				else if (rv == 1)
				{
					D("TLS: accept(%d) = %p\n", fd, c->tls);
					return 1;
				}

#ifdef TLS_IS_OPENSSL
				D("TLS: accept(%d) = failed: %s\n",
				  fd, ERR_error_string(ERR_get_error(), NULL));
#endif

				SSL_free(c->tls);
				c->tls = NULL;
				break;
			}
		}
	}

	return 0;
}
#endif

int uh_tls_client_recv(struct client *c, char *buf, int len)
{
	int rv = SSL_read(c->tls, buf, len);
	int err = SSL_get_error(c->tls, 0);

	if ((rv == -1) && (err == SSL_ERROR_WANT_READ))
	{
		D("TLS: recv(%d, %d) = retry\n", c->fd.fd, len);
		errno = EAGAIN;
		return -1;
	}

	D("TLS: recv(%d, %d) = %d\n", c->fd.fd, len, rv);
	return rv;
}

int uh_tls_client_send(struct client *c, const char *buf, int len)
{
	int rv = SSL_write(c->tls, buf, len);
	int err = SSL_get_error(c->tls, 0);

	if ((rv == -1) && (err == SSL_ERROR_WANT_WRITE))
	{
		D("TLS: send(%d, %d) = retry\n", c->fd.fd, len);
		errno = EAGAIN;
		return -1;
	}

	D("TLS: send(%d, %d) = %d\n", c->fd.fd, len, rv);
	return rv;
}

void uh_tls_client_close(struct client *c)
{
	if (c->tls)
	{
		D("TLS: close(%d)\n", c->fd.fd);

		SSL_shutdown(c->tls);
		SSL_free(c->tls);

		c->tls = NULL;
	}
}
