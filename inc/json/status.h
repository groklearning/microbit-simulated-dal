/**
 * \file
 **/
#pragma once

/**
 * An enum used as the return value for most of our APIs. The different values indicate different
 * non-success conditions.
 **/
enum status {
  STATUS_BAD,           //!< An unspecified error occurred.
  STATUS_OK,            //!< Everything succeeded.
  STATUS_ENOMEM,        //!< A memory allocation failed.
  STATUS_EINVAL,        //!< An invalid value was provided.
  STATUS_DISCONNECTED,  //!< A remote connection has disconnected.
};
