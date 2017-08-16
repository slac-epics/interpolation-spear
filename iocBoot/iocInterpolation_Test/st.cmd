#!../../bin/linux-x86/Interpolation_Test

## You may have to change Interpolation_Test to something else
## everywhere it appears in this file

< envPaths

cd ${TOP}

## Register all support components
dbLoadDatabase("dbd/Interpolation_Test.dbd",0,0)
Interpolation_Test_registerRecordDeviceDriver(pdbbase)

## Load record instances
#
dbLoadTemplate ("db/Feed_Forward_Sector_03.substitutions")

dbLoadTemplate ("db/Feed_Forward_Sector_05.substitutions")

#dbLoadTemplate ("db/Feed_Forward_Sector_08.substitutions")

#dbLoadTemplate ("db/Feed_Forward_Sector_12.substitutions")

dbLoadTemplate ("db/Feed_Forward_Sector_13.substitutions")

# Sector 14 is asymetric.
#
dbLoadRecords  ("db/Feed_Forward_Sector_14.db")
dbLoadTemplate ("db/Feed_Forward_Sector_14.substitutions")

cd ${TOP}/iocBoot/${IOC}
iocInit()

# end
