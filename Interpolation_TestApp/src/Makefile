# $File: //ASP/Dev/SBS/4_Controls/4_4_Equipment_Control/Common/sw/Interpolation/Interpolation_TestApp/src/Makefile $
# $Revision: 1.1.1.1 $
# $DateTime: 2010/04/22 10:55:31 $
# Last checked in by: $Author: saa $
#

TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# build an ioc application

PROD_IOC = Interpolation_Test

DBD += Interpolation_Test.dbd
# Interpolation_Test.dbd will be made up from these files:
Interpolation_Test_DBD += base.dbd
#include definitions for any other support applications needed
#
Interpolation_Test_DBD += dynamic_interpolation_subroutines.dbd

# <name>_registerRecordDeviceDriver.cpp will be created from <name>.dbd
Interpolation_Test_SRCS += Interpolation_Test_registerRecordDeviceDriver.cpp
Interpolation_Test_SRCS_DEFAULT += Interpolation_TestMain.cpp
Interpolation_Test_SRCS_vxWorks += -nil-

#The following adds support from base/src/vxWorks
Interpolation_Test_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

#add a definition for each support application used by this application
#
Interpolation_Test_LIBS += dynamic_interpolation


Interpolation_Test_LIBS += $(EPICS_BASE_IOC_LIBS)


#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

# end
