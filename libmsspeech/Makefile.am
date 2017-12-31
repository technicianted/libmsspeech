#######################################
# The list of libraries we are building seperated by spaces.
# The 'lib_' indicates that these build products will be installed
# in the $(libdir) directory. For example /usr/lib
lib_LTLIBRARIES = libmsspeech.la

#######################################
# Build information for each library

# Sources for libTest
libmsspeech_la_SOURCES = client_messages.c message_constants.c ms_speech_guid.c ms_speech_logging.c ms_speech_status_control.c ms_speech_telemetry.c ms_speech_timestamp.c ms_speech.c response_messages.c compat.c

# Linker options libTestProgram
libmsspeech_la_LDFLAGS = 

# Compiler options. Here we are adding the include directory
# to be searched for headers included in the source code.
libmsspeech_la_CPPFLAGS = -I$(top_srcdir)/include -std=c99 -D_GNU_SOURCE

