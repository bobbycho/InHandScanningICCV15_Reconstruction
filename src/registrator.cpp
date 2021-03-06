// Author: Dimitrios Tzionas
//
// source code for the work:
//
// Dimitrios Tzionas and Juergen Gall
// 3D Object Reconstruction from Hand-Object Interactions
// International Conference on Computer Vision (ICCV) 2015
// http://files.is.tue.mpg.de/dtzionas/In-Hand-Scanning


#include "registrator.h"



Registrator::Registrator( QString fileName_CONFIG_RUN, QString fileName_CONFIG_PARAMs, bool FLAG_touchFrames_PRINT, bool FLAG_print_CONFIG )
{

        QDir                                qdir( qApp->applicationDirPath() );
                                            qdir.cdUp();
        if (IS_ROOT_DIR(qdir) == false)     qdir.cdUp();

        PATH_CONFIG_baseInput = qdir.path();
        PATH_CONFIG_RUN       = PATH_CONFIG_baseInput + "/config/" + fileName_CONFIG_RUN;
        PATH_CONFIG_PARAMs    = PATH_CONFIG_baseInput + "/config/" + fileName_CONFIG_PARAMs;
        PATH_CONFIG_PATHs     = PATH_CONFIG_baseInput + "/config/" + "/CONFIG_PATHs.txt";
        PATH_CONFIG_DBG       = PATH_CONFIG_baseInput + "/config/" + "/CONFIG_DBG.txt";

        ////////////////////
        read_CONFIG_PATHs();
        ////////////////////

        //////////////////      ///////////////////////////////////////////
        read_CONFIG_RUN();      if (FLAG_print_CONFIG)  print_CONFIG_RUN();
        //////////////////      ///////////////////////////////////////////

        construct( PARAM_RUN_applyHANDs,
                   PARAM_RUN_hasHANDs,
                   PARAM_RUN_INPUT_PATH,
                   PARAM_RUN_RunningMODE,
                   PARAM_RUN_VIEW_ENABLED,
                   PARAM_RUN_FEAT_ENABLED,
                   PARAM_RUN_ICP_ENABLED,
                   PARAM_RUN_syntheticORrealistic,
                   PARAM_RUN_commentStr,
                   FLAG_touchFrames_PRINT,
                   FLAG_print_CONFIG);

}

Registrator::Registrator(    bool applyHANDs_IN, bool hasHANDs_IN, QString INPUT_PATH, QString RunningMODE, bool VIEW_ENABLED, bool FEAT_ENABLED, bool ICP_ENABLED, QString syntheticORrealistic_IN, QString commentStr, bool FLAG_touchFrames_PRINT, bool FLAG_print_CONFIG )
{
                  construct(      applyHANDs_IN,      hasHANDs_IN,         INPUT_PATH,         RunningMODE,      VIEW_ENABLED,      FEAT_ENABLED,      ICP_ENABLED,         syntheticORrealistic_IN,         commentStr,      FLAG_touchFrames_PRINT,      FLAG_print_CONFIG );
}

void Registrator::construct( bool applyHANDs_IN, bool hasHANDs_IN, QString INPUT_PATH, QString RunningMODE, bool VIEW_ENABLED, bool FEAT_ENABLED, bool ICP_ENABLED, QString syntheticORrealistic_IN, QString commentStr, bool FLAG_touchFrames_PRINT, bool FLAG_print_CONFIG )
{

            ////////////////////////////
            ////////////////////////////
              hasHANDs  =   hasHANDs_IN;
            applyHANDs  = applyHANDs_IN;
            applyHANDs &=   hasHANDs;
            ////////////////////////////
            ////////////////////////////


            // http://stackoverflow.com/questions/24071892/suppress-cout-output-from-pcl-registration
            // http://docs.pointclouds.org/trunk/namespacepcl_1_1console.html#a9aaacc5ccdc4afd16cf13ecff7c1b88e
            // http://docs.pointclouds.org/trunk/print_8h_source.html
            // http://docs.pointclouds.org/1.7.0/namespacepcl_1_1console.html
            pcl::console::setVerbosityLevel( pcl::console::L_ALWAYS );
            //enum VERBOSITY_LEVEL
            //{
            //    L_ALWAYS  // turns everything off
            //    L_ERROR   //
            //    L_WARN    //
            //    L_INFO    //
            //    L_DEBUG   //
            //    L_VERBOSE //
            //};


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            QString                                         input__IMG__PCL;
            if (syntheticORrealistic_IN == "synthetic")     input__IMG__PCL = "PCL"; // SYNTHetic -> PCL only -> no camera matrices to backProject !!!
            else                                            input__IMG__PCL = "IMG";

            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if ( INPUT_PATH.endsWith("/") == false )
                 INPUT_PATH +=       "/";
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if ( input__IMG__PCL != "IMG" &&
                 input__IMG__PCL != "PCL"  )
            {
                 std::cout << "\n\n\n" << "PROBLEM with input__IMG__PCL !!!!!  -  input__IMG__PCL = " << input__IMG__PCL.toStdString() << "\t\t" << std::endl;
                 exit(1);
            }            
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            QString          pathhh_RGB = INPUT_PATH + "rgb/";
            QDir recoredDir( pathhh_RGB);

            if (recoredDir.exists() == false)
            {
                std::cout << "\n\n" << pathhh_RGB.toStdString() << "\n\n" << std::endl;

                std::cout << "\n\n" << "Probably you forgot to edit the **./configCONFIG_PATHs.txt** file !!!" << "\n\n" << std::endl;
            }

            QStringList allFiles = recoredDir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst);//(QDir::Filter::Files,QDir::SortFlag::NoSort)

            QStringList myStringListFILENAME = QString(allFiles[0]).split( '.', QString::SkipEmptyParts );

            fileNamePADDING = myStringListFILENAME.at(0).length();

            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

                 syntheticORrealistic = syntheticORrealistic_IN;

            if ( syntheticORrealistic!="synthetic"         &&
                 syntheticORrealistic!="realistic_CARMINE"  )
            {
                 std::cout << std::endl << "SKATA ARGUMENTS - PARAM_RUN_syntheticORrealistic !!!" << std::endl << std::endl;
                 exit(1);
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            if (hasHANDs)
            {

                    if (syntheticORrealistic == "synthetic")
                    {
                                                                                                                            sequenceID = -1;
                            if      ( INPUT_PATH.toLower().contains("seq5") || INPUT_PATH.toLower().contains("seq_5") )     sequenceID =  5;
                            else if ( INPUT_PATH.toLower().contains("seq6") || INPUT_PATH.toLower().contains("seq_6") )     sequenceID =  6;

                    }
                    else // if (syntheticORrealistic.contains("realistic"))
                    {

                            QStringList                   myStringListFromFile = INPUT_PATH.split(QRegExp("/"), QString::SkipEmptyParts);
                            QString      sequenceID_str = myStringListFromFile.at(myStringListFromFile.length()-2); // /media/381279A834F7273C/Luca_Files/Videos_Frames/801/1/
                            ////////////////////////////////////
                            sequenceID = sequenceID_str.toInt();
                            ////////////////////////////////////

                    }


                    if (sequenceID == -1)
                    {
                        std::cout << "\n\n" << "sequenceID == -1" << "\n" << std::endl;
                        exit(1);
                    }


            } // if (applyHANDs)


            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            if (hasHANDs)
            {

                    /***********************/      /*******************************************/
                    /***********************/      /*******************************************/
                    update_PATHs(sequenceID);      if (FLAG_print_CONFIG)  update_PATHs_print();
                    /***********************/      /*******************************************/
                    /***********************/      /*******************************************/

                    QString                                                               sequenceIDstring = QString::number(sequenceID).rightJustified(3,'0',false);
                    QString                                     INPUT_dynamicStringPart;
                    if (syntheticORrealistic == "synthetic")    INPUT_dynamicStringPart = sequenceIDstring + "/";
                    else             /*.contains(realistic)*/   INPUT_dynamicStringPart = sequenceIDstring + "/";


                    /**********************/
                    /**********************/
                    sequence = new Sequence(  //CAMERASET
                    /**********************/     PATH_CAMERA_SET,           // QString INPUT_PATH_CAMERA_SET_IN     // OK
                    /**********************/   //MODELSET
                                                 PATH_OUTPUT_BASE,          // QString INPUT_BasePath
                                                 INPUT_dynamicStringPart,   // QString INPUT_dynamicStringPart
                                                 QString(".OFF"),           // QString INPUT_EXTENSSS_Mesh_IN
                                                 QString(".SKEL"),          // QString INPUT_EXTENSSS_Skeleton
                                                 QString(".SKIN"),          // QString INPUT_EXTENSSS_Skin
                                                 PATH_MODELS_INFO,          // QString INPUT_PATH_MODELS_INFO
                                               //ANIM
                                                 PATH_OUTPUT_BASE,          // PATH_OutputBase
                                                 INPUT_dynamicStringPart,   // PATH_FolderName_INPUT
                                                 QString(".MOTION"),        // INPUT_EXTENSSS_Motion
                                                 PATH_INDEX_BOUNDS,         // PATH_INDEX_BOUNDS_IN
                                               //SEQUENCE
                                                 PATH_INDEX_BOUNDS,         // PATH_INDEX_BOUNDS_INNN,
                                                 sequenceIDstring,          // RadioSequenceID_String_IN
                                                 true   );                  // printEnabled


                    if      ( INPUT_PATH.toLower().contains("cam1") || INPUT_PATH.toLower().contains("cam_1") )     sequence->cameraSet.switchCamera(0);
                    else if ( INPUT_PATH.toLower().contains("cam2") || INPUT_PATH.toLower().contains("cam_2") )     sequence->cameraSet.switchCamera(1);
                    else if ( INPUT_PATH.toLower().contains("cam3") || INPUT_PATH.toLower().contains("cam_3") )     sequence->cameraSet.switchCamera(2);
                    else if ( INPUT_PATH.toLower().contains("cam4") || INPUT_PATH.toLower().contains("cam_4") )     sequence->cameraSet.switchCamera(3);
                    else if ( INPUT_PATH.toLower().contains("cam5") || INPUT_PATH.toLower().contains("cam_5") )     sequence->cameraSet.switchCamera(4);
                    else if ( INPUT_PATH.toLower().contains("cam6") || INPUT_PATH.toLower().contains("cam_6") )     sequence->cameraSet.switchCamera(5);
                    else if ( INPUT_PATH.toLower().contains("cam7") || INPUT_PATH.toLower().contains("cam_7") )     sequence->cameraSet.switchCamera(6);
                    else if ( INPUT_PATH.toLower().contains("cam8") || INPUT_PATH.toLower().contains("cam_8") )     sequence->cameraSet.switchCamera(7);


                    /**********************************/
                    /**********************************/
                    touch    = new Touch(    sequence );
                    renderer = new Renderer( sequence );
                    /**********************************/
                    /**********************************/


            } // if (applyHANDs)

            /////////////////////////////////////////////
            PARAM_applySKIN_2_PCL_backProjection = false;
            /////////////////////////////////////////////

            if (applyHANDs||hasHANDs) // do it for both *synthetic* and *realistic* due to **feat2d***
            {
                    skinnDetector.createSkinModel_JONES_REHG();
                    ////////////////////////////////////////////
                    PARAM_applySKIN_2_PCL_backProjection = true;
                    ////////////////////////////////////////////

                    std::cout << "PARAM_applySKIN_2_PCL_backProjection = "
                              <<  PARAM_applySKIN_2_PCL_backProjection << std::endl;
            }

            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            /*******************/       /********************************************/
            /*******************/       /********************************************/
            read_CONFIG_PARAMs();       if (FLAG_print_CONFIG)  print_CONFIG_PARAMs();                  // includes **touch**
            read_CONFIG_DBG();          if (FLAG_print_CONFIG)  print_CONFIG_DBG();
            /*******************/       /********************************************/
            /*******************/       /********************************************/


            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            if (hasHANDs   &&   syntheticORrealistic.contains("realistic"))
            {
                PARAM_CAM_Intrinsics_fx = sequence->cameraSet.cameras[ sequence->cameraSet.currentCameraID ].KKK(0,0);
                PARAM_CAM_Intrinsics_fy = sequence->cameraSet.cameras[ sequence->cameraSet.currentCameraID ].KKK(1,1);
                PARAM_CAM_Intrinsics_px = sequence->cameraSet.cameras[ sequence->cameraSet.currentCameraID ].KKK(0,2);
                PARAM_CAM_Intrinsics_py = sequence->cameraSet.cameras[ sequence->cameraSet.currentCameraID ].KKK(1,2);
            }


            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            bool                                  TEST_MODE = false;
            if (RunningMODE == "local_TEST")   {  TEST_MODE = true;
                RunningMODE =  "local";        }
            //////////////////////////////////////////////////
            PARAM_DBG_flags__regulateActivation(  TEST_MODE );
            //////////////////////////////////////////////////


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            QString basicFolder = commentStr + "/";

            QDir                         qdir( INPUT_PATH );
                                         qdir.cdUp();
            if ( QDir(         QString(  qdir.path() + "/" + "RESULTS/" )).exists() == false)
                 QDir().mkdir( QString(  qdir.path() + "/" + "RESULTS/" ) );

                           ///////////
            QString        OUTPUT_PATH = qdir.path() + "/" + "RESULTS/" + basicFolder;
                           ///////////


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            int NUMBBB;


            if (RunningMODE != "touchIDs")
            {

                    if (TEST_MODE == false)
                    {
                        if ( QDir(  QString(  INPUT_PATH + "contactFrameIDs/"                  ) ).exists() == false)   {   std::cout << "\n\n" << "No **"<<QString(  INPUT_PATH + "contactFrameIDs/"                  ).toStdString()<<"** folder !!!" << "\n\n" << std::endl;   exit(1);   }
                        if ( QFile( QString(  INPUT_PATH + "contactFrameIDs/" + "index.pairs"  ) ).exists() == false)   {   std::cout << "\n\n" << "No **"<<QString(  INPUT_PATH + "contactFrameIDs/" + "index.pairs"  ).toStdString()<<"** file !!!"   << "\n\n" << std::endl;   exit(1);   }
                    }

                                                    touchIndices_READ( INPUT_PATH + "contactFrameIDs/" + "index.pairs" );
                                                    touchFrames_from_touchIndices_COMPUTE();
                    if (FLAG_touchFrames_PRINT)     touchFrames_PRINT();

                    ////////
                    NUMBBB = PARAM_INDEX_List.frames.size();
                    ////////

            }
            else
            {

                    QDir                    currDir = INPUT_PATH + "rgb";
                                            currDir.setNameFilters( QStringList()<<("*.png") );
                    QStringList  fileList = currDir.entryList();

                    ////////
                    NUMBBB = fileList.size();
                    ////////

                    std::cout <<"\n"<<currDir.path().toStdString() << " - " << "PCLs = " << NUMBBB<<"\n"<< std::endl;

            }


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            if (applyHANDs)
            {

                    for (int animNumb=0; animNumb<sequence->posedAnimations.size(); animNumb++)
                    {

                            for (int modelID=0; modelID<sequence->posedAnimations[animNumb].modelSet.totalModels; modelID++)
                            {

                                    Model* pModel = &(sequence->posedAnimations[animNumb].modelSet.models[modelID]);

                                    if      (pModel->name    ==    "Model_Hand_R")      pModel->isRenderable = PARAM_DBG_Renderable_Model_Hand_R;
                                    else if (pModel->name    ==    "Model_Hand_L")      pModel->isRenderable = PARAM_DBG_Renderable_Model_Hand_L;
                                    else   /*pModel->name.contains("Model_Object")*/    pModel->isRenderable = PARAM_DBG_Renderable_Model_Object;

                            }

                    }

            }


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            if (applyHANDs && FEAT_ENABLED==false)
            {
                PARAM_RUN_CorrWeight_OBJ_Feat2d = 0.0;
                PARAM_RUN_CorrWeight_OBJ_Feat3d = 0.0;
            }


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            std::cout <<                                                   std::endl;
            std::cout << "********************************************" << std::endl;
            std::cout << "RunningMODE => " << RunningMODE.toStdString() << std::endl;
            std::cout << "********************************************" << std::endl;
            std::cout <<                                                   std::endl;            

            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            //////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////
            int                                                                 offHand =   0;
            if (syntheticORrealistic == "synthetic")                            //////////////
            {                                                                   //////////////
                if      (INPUT_PATH.contains("24__180"))                        offHand = 179;
                else if (INPUT_PATH.contains("86__270"))                        offHand = 269;
            } ////////////////////////////////////////////////////////////////////////////////
            else
            {
                if (INPUT_PATH.contains("SYMM___ball_green_faceee_handsYES"))   offHand = 45;
            }
            //////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            if (FEAT_ENABLED && (PARAM_RUN_CorrWeight_OBJ_Feat2d>0))
            {
                    rmatcher.setPARAMs( PARAM_SIFT_KEY_nFeatures,
                                        PARAM_SIFT_KEY_nOctaveLayers,
                                        PARAM_SIFT_KEY_contrastThreshold,
                                        PARAM_SIFT_KEY_edgeThreshold,
                                        PARAM_SIFT_KEY_sigma,
                                        fileNamePADDING);
            }


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            if (applyHANDs==true && PARAM_RUN_CorrWeight_SKIN<=0)
            {
                std::cout << "\n\n" << "** applyHANDs==true && PARAM_RUN_CorrWeight_SKIN<=0 **" << "\n\n" << std::endl;
                exit(1);
            }


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


            if (applyHANDs==false)      PARAM_RUN__ONLY__TOUCH__TRANSF = false;

            if (TEST_MODE)              PARAM_RUN__ONLY__TOUCH__TRANSF = false;


            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            else if (RunningMODE == "local")                myReg_1_LocalAlign(    TEST_MODE, INPUT_PATH, OUTPUT_PATH, NUMBBB, VIEW_ENABLED, FEAT_ENABLED, ICP_ENABLED, input__IMG__PCL, offHand );
            else if (RunningMODE == "tsdf_local")           myReg_X_TSDF_local(               INPUT_PATH, OUTPUT_PATH, NUMBBB, VIEW_ENABLED,                            input__IMG__PCL, true    );
            else if (RunningMODE == "meshlab_local")        myReg_X_MESHLAB_local(                        OUTPUT_PATH);
            else
            {
                std::cout << "\n\n" << "INVALID RunningMODE !!! -> " << RunningMODE.toStdString() << "\n" << std::endl;
                exit(1);
            }

            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            if (applyHANDs)
            {
                    ////////////////
                    delete sequence;
                    delete touch;
                    delete renderer;
                    ////////////////
            }

}







bool Registrator::IS_ROOT_DIR( QDir &qdir )
{

        QStringList allFiles = qdir.entryList(QDir::NoDotAndDotDot | QDir::System | QDir::AllDirs | QDir::Files, QDir::DirsFirst);//(QDir::Filter::Files,QDir::SortFlag::NoSort)   QDir::Hidden  |

        //std::cout << "myStringListFILENAME.count() = " << allFiles.count() << "     " << qdir.path().toStdString() << std::endl;

        bool FLAG_ROOT_Dir = false;
        bool FLAG_config   = false;
        bool FLAG_src      = false;

        for (int iii=0; iii<allFiles.count(); iii++)
        {
            //std::cout << allFiles[iii].toStdString() << std::endl;

            if      (allFiles[iii] == "src")       FLAG_src    = true;
            else if (allFiles[iii] == "config")    FLAG_config = true;
        }

        FLAG_ROOT_Dir = FLAG_src && FLAG_config;

        return FLAG_ROOT_Dir;

}


