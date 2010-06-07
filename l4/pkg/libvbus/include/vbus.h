/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU General Public License 2.
 * Please see the COPYING-GPL-2 file for details.
 */
#pragma once

#include <l4/sys/compiler.h>
#include <l4/vbus/vbus_types.h>
#include <l4/sys/types.h>

enum {
  L4VBUS_NULL = 0,
  L4VBUS_ROOT_BUS = 0,
};

__BEGIN_DECLS

/**
 * \brief Find a device by the HID APCI conforming or L4Io static name.
 *
 * \param  vbus		capability of the system bus
 * \param  parent	handle to the parent to start the search
 * \retval child	handle to the found device
 * \param  hid		HID name of the device
 * \retval devinfo      Device information structure (might be NULL)
 * \retval reshandle    Resource handle (might be NULL)
 *
 * \return 0 on succes, else failure
 */
int L4_CV
l4vbus_get_device_by_hid(l4_cap_idx_t vbus, l4vbus_device_handle_t parent,
                         l4vbus_device_handle_t *child, char const *hid,
                         int depth, l4vbus_device_t *devinfo);

/**
 * \brief Find next child following \a child.
 *
 * \param  vbus		capability of the system bus
 * \param  parent	handle to the parent device (use 0 for the system bus)
 * \param  child	handle to the child device (use 0 to get the first
 *                      child)
 * \retval next		handle to the successor of child
 * \retval devinfo      device information (might be NULL)
 *
 * \return 0 on succes, else failure
 */
int L4_CV
l4vbus_get_next_device(l4_cap_idx_t vbus, l4vbus_device_handle_t parent,
                       l4vbus_device_handle_t *child, int depth,
                       l4vbus_device_t *devinfo);

/**
 * \brief Iterate over the resources of a device
 *
 * \param  vbus		capability of the system bus
 * \param  dev		handle of the device
 * \retval res_idx	Index of the resource, the number of resources is
 * 			availabnle in the devinfo from get device functions.
 * \retval res		descriptor of the resource
 *
 * \return 0 on succes, else failure
 */
int L4_CV
l4vbus_get_resource(l4_cap_idx_t vbus, l4vbus_device_handle_t dev,
                    int res_idx,
                    l4vbus_resource_t *res);

/**
 * \brief Request a resource of a specific type
 *
 * \param  vbus		capability of the system bus
 * \param  res		descriptor of the resource
 * \param  flags	optional flags
 *
 * \return 0 on succes, else failure
 *
 * If any resource is found that contains the requested
 * type and addresses this resource is returned.
 *
 * Flags are only relevant to control the memory caching.
 * If io-memory is requested.
 *
 * \return 0 on succes, else failure
 */
int L4_CV
l4vbus_request_resource(l4_cap_idx_t vbus, l4vbus_resource_t *res,
                        int flags);

/**
 * \brief Release a previously requested resource
 *
 * \param  vbus		capability of the system bus
 * \param  res		descriptor of the resource
 *
 * \return 0 on succes, else failure
 */
int L4_CV
l4vbus_release_resource(l4_cap_idx_t vbus, l4vbus_resource_t *res);

int L4_CV
l4vbus_vicu_get_cap(l4_cap_idx_t vbus, l4vbus_device_handle_t icu,
                    l4_cap_idx_t res);

__END_DECLS
