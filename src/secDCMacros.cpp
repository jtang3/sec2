#include "secDCMacros.hh"
#include "secDetectorConstruction.hh"

#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4PhysicalVolumeStore.hh"

#include "G4UIcmdWithoutParameter.hh"
#include "G4UIcmdWithABool.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWith3VectorAndUnit.hh"
#include "G4UIcmdWith3Vector.hh"

#include "G4ThreeVector.hh"
#include "G4RotationMatrix.hh"

secDCMacros::secDCMacros(secDetectorConstruction* secDC) :
    DetectorConstruction(secDC),
    LVNow(nullptr),
    PVNow(nullptr)
    IsOverLapCheck(true)
{
/*
    create commands for detector construction. ALL COMMAND BEGINS WITH /sec/DC
*/
    //command for toggling the overlap checking
    cmd_OverLapCheck = new G4UIcmdWithABool("/sec/DC/IsOverLapCheck", this);
    cmd_OverLapCheck->SetGuidence("Enable/Disable Detector Overlap Checking");
    cmd_OverLapCheck->SetParameterName("Flag", false); 
    cmd_OverLapCheck->SetDefaultValue(true);

    //command for specifing the logical volume
    cmd_SpecifyLV = new G4UIcmdWithAString("/sec/DC/LogicalVolume", this);
    cmd_SpecifyLV->SetGuidence("Specify the LogicalVolume to further set parmeter");
    cmd_SpecifyLV->SetParameterName("LogicalVolumeName");

    //command for specifing the logical volume
    cmd_SpecifyPV = new G4UIcmdWithAString("/sec/DC/PhysicalVolume", this);
    cmd_SpecifyPV->SetGuidence("Specify the Solid to further set parameter");
    cmd_SpecifyPV->SetGuidence("PhysicalVolumeName");

    //command for loading the data files for logical volume
    cmd_FileName = new G4UIcmdWithAString("/sec/DC/LoadFileForLV");
    cmd_FileName->SetGuidence("Specify the data file's name");
    cmd_FileName->SetParameterName("FileName");

    //command for translating the Physical Volume
    cmd_PVPos = new G4UIcmdWith3VectorAndUnit("/sec/DC/Translation");
    cmd_PVPos->SetGuidence("Translate the Physical Volume to a new position");
    cmd_PVPos->SetParameterName("PosX", "PosY", "PosZ", false);

    //command for rotating the Physical Volume
    cmd_PVRotate = new G4UIcmdWith3Vector("/sec/DC/Rotation");
    cmd_PVRotate->SetGuidence("Rotate the Physical Volume according to Euler Angles (rad)");
    cmd_PVRotate->SetParameterName("alpha", "beta", "gamma", false);
}
secDCMacros::~secDCMacros()
{
    //delete the commands!!
    delete cmd_OverLapCheck;
    delete cmd_SpecifyLV;
    delete cmd_SpecifyPV;
    delete cmd_FileName;
    delete cmd_PVPos;
    delete cmd_PVRotate;
}

void secDCMacros::SetNewValue(G4UIcommand* cmd, G4String NewVal)
{
    if( cmd == cmd_OverLapCheck )
    {
        IsOverLapCheck = cmd->ConvertToBool(NewVal);
    }
    else if( cmd == cmd_SpecifyLV )
    {
        //not the store selling LVs! But the logical volume store!!
        auto LVStore = G4LogicalVolumeStore::GetInstance();
        LVNow = LVStore->GetVolume(NewVal);
        
        // if the volume doesn't exist, always set the volume "world_log"( world volume of the detector )
        if(LVNow = nullptr)
        {
            LVStore->GetVolume("world_log");
            std::cerr << "===========================================================\n"
                      << "                   Warning From sec2"
                      << "Logical volume called " << NewVal << " NOT FOUND!"
                      << "Probably wrong name, current Logical Volume is world volume"
                      << std::endl;
        }
    }
    else if( cmd == cmd_SpecifyPV )
    {
        auto PVStore = G4PhysicalVolume::GetInstance();
        PVNow = PVStore->GetVolume(NewVal);

        if( PVNow = nullptr )
        {
            //always set the volume "world_phy" if the volume doesn't exist.
            PVStore->GetVolume("world_phy");
            if(PVNow = nullptr)
            {
                PVStore->GetVolume("world_log");
                std::cerr << "===========================================================\n"
                            << "                   Warning From sec2"
                            << "Physical volume called " << NewVal << " NOT FOUND!"
                            << "Probably wrong name, current Logical Volume is world volume"
                            << std::endl;
            }
        }
    }
    else if ( cmd == cmd_FileName )
    {
        //unfinished!
        //register the data file in the detector construction
        DetectorConstruction->SetLVOpticalProperties(LVNow, NewVal);
    }
    else if( cmd == cmd_PVPos )
    {
        //Move the Physical Volume to a new place.
        PVNow->SetTranslation( cmd->ConvertTo3Vector(NewVal) );
        //inform the run manager
        InformRunMgr();
        DumpOverLapInfo(IsOverLapCheck, PVNow)
    }
    else if( cmd == cmd_PVRotate )
    {
        //rotate the Physical Volume accoring to the Euler Angle.
        G4ThreeVector EulerVect = cmd->ConvertTo3Vector(NewVal);
        PVNow->SetObjectRotation( G4RotationMatrix(EulerVect.x(),
                                                   EulerVect.y(), 
                                                   EulerVect.z()) );
        //inform the run manager
        InformRunMgr();
        DumpOverLapInfo(IsOverLapCheck, PVNow);
    }    
}
