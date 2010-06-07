/**
 * \file    ddekit/include/config.h
 * \brief   Configuration file for ddekit.
 */

#pragma once

#define DEBUG_MSG(msg, ...) ddekit_printf("%s: \033[32;1m"msg"\033[0m\n", __FUNCTION__, ##__VA_ARGS__)
