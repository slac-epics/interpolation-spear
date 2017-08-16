# $File: //ASP/Dev/SBS/4_Controls/4_4_Equipment_Control/Common/sw/Interpolation/Dynamic_InterpolationSup/Makefile $
# $Revision: 1.1.1.1 $
# $DateTime: 2012/02/10 22:07:25 $
# Last checked in by: $Author: saa $
#
TOP=..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#==================================================
# build a support library

LIBRARY_IOC += dynamic_interpolation

DBD += dynamic_interpolation_subroutines.dbd

# specify all source files to be compiled and added to the library
#
dynamic_interpolation_SRCS += dynamic_interpolation.c
dynamic_interpolation_SRCS += dynamic_interpolation_subroutines.c

dynamic_interpolation_LIBS += $(EPICS_BASE_IOC_LIBS)

#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end
