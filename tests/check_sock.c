/**
 * =====================================================================================
 * @file   check_sock.c
 * @brief  test suite for sockutils library
 * @date     08/09/2010
 * @author   Roey Berman, (roey.berman@gmail.com)
 * @version  1.0
 *
 * Copyright (c) 2010, Roey Berman, (roeyb.berman@gmail.com)
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Roey Berman.
 * 4. Neither the name of Roey Berman nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY ROEY BERMAN ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ROEY BERMAN BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <check.h>
#include <fcntl.h>
#include "sockutils.h"

static char errorstr[SOCK_ERRSTR_LEN];

START_TEST(invalid_hostname_t) {
    fail_if(tcp_client("bad hostname", "666", errorstr) != SOCK_ERR);
}
END_TEST

START_TEST(unable_to_connect_t) {
    fail_if(tcp_client("www.walla.co.il", "666", errorstr) != SOCK_ERR);
}
END_TEST

START_TEST(server_client_t) {
    int flags, server, client;
    fail_if((server = tcp_server("0.0.0.0", "16666", 10, errorstr)) == SOCK_ERR);
    fail_if((client = tcp_client("0.0.0.0", "16666", errorstr)) == SOCK_ERR);
    fail_if(( flags = fcntl(client, F_GETFL, NULL)) < 0);
    fail_unless(unblock(client, errorstr) && fcntl(client, F_GETFL, NULL) & O_NONBLOCK, "unblock failed");
    fail_unless(unset_sock_flags(client, O_NONBLOCK, errorstr), "failed to unset flags");
    fail_if(flags != fcntl(client, F_GETFL, NULL), "flags were not restored after unset");
}
END_TEST

Suite *local_suite(void)
{
    Suite *s  = suite_create(__FILE__);
    TCase *tc = tcase_create(__FILE__);

    tcase_add_test(tc, invalid_hostname_t);
    tcase_add_test(tc, unable_to_connect_t);
    tcase_add_test(tc, server_client_t);

    suite_add_tcase(s, tc);
    return s;
}

int main() {
    SRunner *sr;
    Suite *s;
    int failed;

    s = local_suite();
    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);

    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
