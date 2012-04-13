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

#ifndef MMSTAFFFILE_H_
#define MMSTAFFFILE_H_

#include "mmstools/mmsfile.h"

//! TAFF Eyecatcher
#define TAFF_IDENT	"TAFF"

//! Types of TAFF attributes
typedef enum {
	//! not set
	TAFF_ATTRTYPE_NONE,
	//! any characters
	TAFF_ATTRTYPE_STRING,
	//! any characters, but not empty
	TAFF_ATTRTYPE_NE_STRING,
	//! valid values: "true", "false"
	TAFF_ATTRTYPE_BOOL,
	//! valid values: "0".."255"
	TAFF_ATTRTYPE_UCHAR,
	//! valid values: "0".."100"
	TAFF_ATTRTYPE_UCHAR100,
	//! valid values: "-2147483648".."2147483647"
	TAFF_ATTRTYPE_INT,
	//! any binary data
	TAFF_ATTRTYPE_BINDATA,
	//! valid values: "true", "false", "auto"
	TAFF_ATTRTYPE_STATE,
	//! valid values: "true", "false", "linear", "log", "log_soft_start", "log_soft_end"
	TAFF_ATTRTYPE_SEQUENCE_MODE,
	//! argb values in hex format, syntax: "#rrggbbaa"
	TAFF_ATTRTYPE_COLOR
} TAFF_ATTRTYPE;

//! Describe a TAFF attribute
typedef struct {
	//! name of attribute
	const char		*name;
	//! type of attribute
	TAFF_ATTRTYPE 	type;
} TAFF_ATTRDESC;

//! Describe a TAFF tag
typedef struct {
	//! name of tag
	const char		*name;
	//! name of special type attribute
	const char		*typeattr;
	//! value of special type attribute
	const char		*type;
	//! attributes
	TAFF_ATTRDESC 	*attr;
} TAFF_TAGTABLE;

//! Describe a TAFF file format
typedef struct {
	//! type of TAFF file
	char			type[32];
	//! type-based version
	unsigned int	version;
	//! tags
	TAFF_TAGTABLE	*tagtable;
} TAFF_DESCRIPTION;

//! Internal tag/attribute identifiers
typedef enum {
	//! internally identifies a close tag
	MMSTAFF_TAGTABLE_TYPE_CLOSETAG,
	//! internally identifies a tag
	MMSTAFF_TAGTABLE_TYPE_TAG,
	//! internally identifies a attribute
	MMSTAFF_TAGTABLE_TYPE_ATTR
} MMSTAFF_TAGTABLE_TYPE;

//! internally identifies attributes without id (attribute names will be stored)
#define MMSTAFF_ATTR_WITHOUT_ID		0xff

//! Supported types of external files
typedef enum {
	//! the external file is written in XML
	MMSTAFF_EXTERNAL_TYPE_XML,
	//! the external file is an image (currently we only support PNG images) */
	MMSTAFF_EXTERNAL_TYPE_IMAGE
} MMSTAFF_EXTERNAL_TYPE;

//! Supported pixelformats of taff images
typedef enum {
	//! 32 bit ARGB (4 byte, alpha 8\@24, red 8\@16, green 8\@8, blue 8\@0)
	MMSTAFF_PF_ARGB,
	//! 32 bit ARGB (4 byte, inv. alpha 8\@24, red 8\@16, green 8\@8, blue 8\@0)
	MMSTAFF_PF_AiRGB,
	//! 32 bit AYUV (4 byte, alpha 8\@24, Y 8\@16, Cb 8\@8, Cr 8\@0)
	MMSTAFF_PF_AYUV,
    //! 16 bit ARGB (2 byte, alpha 4\@12, red 4\@8, green 4\@4, blue 4\@0)
    MMSTAFF_PF_ARGB4444,
    //! 16 bit RGB (2 byte, red 5\@11, green 6\@5, blue 5\@0)
    MMSTAFF_PF_RGB16,
	//! 32 bit ABGR (4 byte, alpha 8\@24, blue 8\@16, green 8\@8, red 8\@0)
	MMSTAFF_PF_ABGR,
} MMSTAFF_PF;

//! convert 4 bytes from byte stream to an 32 bit integer (needed especially by ARM)
#define MMSTAFF_INT32_FROM_UCHAR_STREAM(stream) \
     ((int)( (stream)[0] | ((stream)[1] << 8) | ((stream)[2] << 16) | ((stream)[3] << 24) ))

//! A data access class for Tagged Attributes File Format (TAFF).
/*!
This class is written to generate an simple to parse binary presentation of
high level markup languages such as XML. For now the conversion XML to TAFF
or vice versa and PNG/JPEG/TIFF to TAFF is supported.
The user of this class must specify a description of which tags and attributes
are allowed. Further he specifies the type of an attribute. With this informations
this class also checks the types and ranges of attributes during the conversion.
For example the MMSGUI works completely with TAFF.
\author Jens Schneider
*/
class MMSTaffFile {
	private:
		//! taff filename
		string 				taff_filename;

		//! describe the tags and attributes
		TAFF_DESCRIPTION 	*taff_desc;

		//! binary presentation data
		unsigned char 		*taff_buf;

		//! size of the buffer
		int					taff_buf_size;

		//! current read position
		int					taff_buf_pos;

		//! name of the external file for conversion
		string 					external_filename;

		//! type of the external file
		MMSTAFF_EXTERNAL_TYPE	external_type;

		//! ignore blank values during the conversion from external file
		bool    ignore_blank_values;

		//! print trace messages?
		bool 	trace;

		//!	print warnings?
		bool    print_warnings;

		//! convert image to this pixelformat
		MMSTAFF_PF	destination_pixelformat;

		//! should the destination pixels premultiplied?
		bool		destination_premultiplied;

		//! size of the mirror in pixel
		int mirror_size;

		//! rotate by 180 degree?
		bool rotate_180;

		//! is the TAFF buffer loaded?
		bool	loaded;

		//! has the TAFF file the correct version?
		bool	correct_version;

		//! id of the current tag
		int		current_tag;

		//! buffer postion of the current tag
		int		current_tag_pos;

        //! Internal method: Writes a buffer to a file.
        bool writeBuffer(MMSFile *file, void *ptr, size_t *ritems, size_t size, size_t nitems, bool *write_status = NULL);

        //! Internal method: Create mirror effect, rotate and convert to target pixelformat.
        bool postprocessImage(void **buf, int *width, int *height, int *pitch,
								 int *size, bool *alphachannel);

        //! Internal method: Read a PNG Image.
		bool readPNG(const char *filename, void **buf, int *width, int *height, int *pitch,
						int *size, bool *alphachannel);

        //! Internal method: Read a JPEG Image.
		bool readJPEG(const char *filename, void **buf, int *width, int *height, int *pitch,
						int *size, bool *alphachannel);

        //! Internal method: Read a TIFF Image.
		bool readTIFF(const char *filename, void **buf, int *width, int *height, int *pitch,
						int *size, bool *alphachannel);

		//! Internal method: Recursive called method for XML to TAFF conversion.
        bool convertXML2TAFF_throughDoc(int depth, void *void_node, MMSFile *taff_file);

        //! Internal method: XML to TAFF conversion.
        bool convertXML2TAFF();

        //! Internal method: IMAGE to TAFF conversion.
        bool convertIMAGE2TAFF();

        //! Internal method: Recursive called method for TAFF to XML conversion.
        bool convertTAFF2XML_throughDoc(int depth, int tagid, MMSFile *external_file);

        //! Internal method: TAFF to XML conversion.
        bool convertTAFF2XML();

	public:
        //! Constructor of class MMSTaffFile.
        /*!
        \param taff_filename		under this name the converted TAFF buffer is/will be stored
        \param taff_desc			the user of this class have to support this tag/attribute description, use NULL
                                    here if you use the external type MMSTAFF_EXTERNAL_TYPE_IMAGE
        \param external_filename	name of the external file for conversion
        							set to blank if no conversion is to be done in the constructor
        \param external_type		type of the external file
        \param ignore_blank_values	ignore blank values during the conversion from external file
        \param trace				print trace messages?
        \param print_warnings		print warnings?
        \param force_rewrite_taff	(re-)convert from external file before loading TAFF
        \param auto_rewrite_taff	(re-)convert from external file if the TAFF file is older than the external file
        */
		MMSTaffFile(string taff_filename, TAFF_DESCRIPTION *taff_desc,
        			string external_filename = "", MMSTAFF_EXTERNAL_TYPE external_type = MMSTAFF_EXTERNAL_TYPE_XML,
        			bool ignore_blank_values = false, bool trace = false, bool print_warnings = false,
        			bool force_rewrite_taff = false, bool auto_rewrite_taff = true);

        //! Destructor of class MMSTaffFile.
		~MMSTaffFile();

		//! Convert external file to TAFF.
        bool convertExternal2TAFF();

		//! Convert TAFF to external.
        bool convertTAFF2External();

		//! Read the TAFF file. This will normally done in the constructor.
        bool readFile();

        //! Is TAFF buffer filled?
        /*!
        \return true if successfully loaded
        */
        bool isLoaded();

        //! Has the TAFF file the correct version described in TAFF description?
        /*!
        \return true if correct version
        */
        bool checkVersion();

        //! Set or reset the external file and type.
        /*!
        \param external_filename	name of the external file for conversion
        \param external_type		type of the external file
        */
        void setExternal(string external_filename = "", MMSTAFF_EXTERNAL_TYPE external_type = MMSTAFF_EXTERNAL_TYPE_XML);

        //! Switch trace on/off.
        /*!
        \param trace	print trace messages?
        */
        void setTrace(bool trace);

        //! Switch print warnings on/off.
        /*!
        \param print_warnings	print warnings?
        */
        void setPrintWarnings(bool print_warnings);

        //! Set the final pixelformat for the convertion (type MMSTAFF_EXTERNAL_TYPE_IMAGE).
        /*!
        \param pixelformat		final pixelformat
        \param premultiplied	the image will be premultiplied during the conversion
        */
        void setDestinationPixelFormat(MMSTAFF_PF pixelformat = MMSTAFF_PF_ARGB, bool premultiplied = true);

        //! Set the mirror effect (type MMSTAFF_EXTERNAL_TYPE_IMAGE).
        /*!
        \param size	size of the mirror effect in pixel
        */
        void setMirrorEffect(int size);

        //! Rotate the image by 180 degree (type MMSTAFF_EXTERNAL_TYPE_IMAGE).
        void rotate180(bool rotate_180);

        //! Get the first tag id.
        /*!
        \return id of the tag or -1 if an error has occurred
        */
        int  getFirstTag();

        //! Get the next tag id.
        /*!
        \param eof	if eof set to true after calling this method, the end of file is reached
        \return id of the tag or -1 in case of close tag or eof
        */
        int  getNextTag(bool &eof);

        //! Get the id of the current tag.
        /*!
        \param name		optional, with this parameter you can get the name of the current tag
        \return id of the current tag
        */
        int  getCurrentTag(const char **name = NULL);

        //! Get the name of the current tag.
        /*!
        \return name of the current tag
        */
        const char *getCurrentTagName();

        //! Copy the complete current tag into a new MMSTaffFile.
        /*!
        \return pointer to the new MMSTaffFile or NULL in case of errors
        */
        MMSTaffFile *copyCurrentTag();

        //! Determine if the current tag has attributes.
        /*!
        \return true if the current tag has at least one attribute
        */
        bool  hasAttributes();

        //! Get the first attribute of the current tag.
        /*!
        \param value_str	return pointer to null terminated value string or NULL if value is returned by value_int parameter
        \param value_int	a few types of attributes will be directly stored in binary format and will be returned by this parameter
        \param name			optional, with this parameter you can get the name of the attribute
        \return id of the attribute or -1 in case of close tag has reached
        */
        int  getFirstAttribute(char **value_str, int *value_int, char **name = NULL);

        //! Get the next attributes of the current tag.
        /*!
        \param value_str	return pointer to null terminated value string or NULL if value is returned by value_int parameter
        \param value_int	a few types of attributes will be directly stored in binary format and will be returned by this parameter
        \param name			optional, with this parameter you can get the name of the attribute
        \return id of the attribute or -1 in case of close tag has reached
        */
        int  getNextAttribute(char **value_str, int *value_int, char **name = NULL);

        //! Searching for an attribute id within the current tag.
        /*!
        \param id           attribute id to search for
        \param value_str	return pointer to null terminated value string or NULL if value is returned by value_int parameter
        \param value_int	a few types of attributes will be directly stored in binary format and will be returned by this parameter
        \return true if attribute was found
        */
        bool getAttribute(int id, char **value_str, int *value_int);

        //! Searching for an attribute id within the current tag.
        /*!
        \param id  	attribute id to search for
        \return pointer to null terminated value string or NULL
        */
        char *getAttributeString(int id);

        //! Convert a value given as string into binary format and check ranges.
        /*!
        \param attrType  			type of the attribute value string
        \param attrValStr			attribute value string
        \param attrValStr_valid		returns if attribute value string is valid
        \param int_val_set			returns true if the value is an integer (int)
        \param byte_val_set			returns true if the value is an byte (unsigned char)
        \param int_val				binary presentation of the value if int_val_set or byte_val_set set to true
        \param attrname				optional attribute name needed for error messages
        \param attrid				optional attribute id needed for error messages
        \param nodename				optional tag name needed for error messages
        \param nodeline				optional line needed for error messages
        \return true if conversion was successful
        \note If method returns true the attrValStr can be invalid anyway (see attrValStr_valid)
        */
        bool convertString2TaffAttributeType(TAFF_ATTRTYPE attrType, char *attrValStr, bool *attrValStr_valid,
											 bool *int_val_set, bool *byte_val_set, int *int_val,
											 const char *attrname = NULL, int attrid = -1, const char *nodename = 0, int nodeline = -1);

};


namespace MMSTAFF_IMAGE_RAWIMAGE_ATTR {

	#define MMSTAFF_IMAGE_RAWIMAGE_ATTR_ATTRDESC \
		{ "width", TAFF_ATTRTYPE_INT }, \
		{ "height", TAFF_ATTRTYPE_INT }, \
		{ "pitch", TAFF_ATTRTYPE_INT }, \
		{ "size", TAFF_ATTRTYPE_INT }, \
		{ "data", TAFF_ATTRTYPE_BINDATA }, \
		{ "pixelformat", TAFF_ATTRTYPE_INT }, \
		{ "premultiplied", TAFF_ATTRTYPE_BOOL }, \
		{ "mirror_size", TAFF_ATTRTYPE_INT }, \
		{ "alphachannel", TAFF_ATTRTYPE_BOOL }, \
		{ "rotate_180", TAFF_ATTRTYPE_BOOL }

	#define MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_width, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_height, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_pitch, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_size, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_data, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_pixelformat, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_premultiplied, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_mirror_size, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_alphachannel, \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_rotate_180

	#define MMSTAFF_IMAGE_RAWIMAGE_ATTR_INIT { \
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_ATTRDESC, \
		{NULL, TAFF_ATTRTYPE_NONE} \
	}

	typedef enum {
		MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS
	} ids;
}

extern TAFF_DESCRIPTION mmstaff_image_taff_description;

typedef enum {
	MMSTAFF_IMAGE_TAGTABLE_TAG_RAWIMAGE
} MMSTAFF_IMAGE_TAGTABLE_TAG;


#endif /*MMSTAFFFILE_H_*/
