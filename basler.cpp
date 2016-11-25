#include "basler.h"

basler::basler(MainWindow *mainwindow)
{
    Main = mainwindow;
    PylonInitialize();
}

bool basler::connect()
{
    // Before using any pylon methods, the pylon runtime must be initialized.


    try
    {
        // Create an instant camera object with the camera device found first.
        camera = new CInstantCamera(CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
        cout << "Using device " << camera->GetDeviceInfo().GetModelName() << endl;

        // The parameter MaxNumBuffer can be used to control the count of buffers
        // allocated for grabbing. The default value of this parameter is 10.
        camera->MaxNumBuffer = 10;
    }    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
             << e.GetDescription() << endl;
        return false;
        //exitCode = 1;
    }

    return true;

}

void basler::closeAllCameras()
{
    try
    {
    if(camera != NULL)
        if(camera->IsOpen())
            camera->Close();
    }
    catch(const GenericException &e)
    {
        qDebug()<<e.GetDescription();
    }

    try
    {
    if(Camera != NULL)
        if(Camera->IsOpen())
            Camera->Close();
    }
    catch(const GenericException &e)
    {

    }
}

void basler::gpio(bool value)
{

    {
        // Get the transport layer factory.
        CTlFactory& TlFactory = CTlFactory::GetInstance();

        // Create the transport layer object needed to enumerate or
        // create a camera object of type Camera_t::DeviceClass().
        ITransportLayer *pTl = TlFactory.CreateTl(Camera_t::DeviceClass());

        // Exit the application if the specific transport layer is not available.
        if (! pTl)
        {
            cerr << "Failed to create transport layer!" << endl;
            //pressEnterToExit();
            return ;
        }

        // Get all attached cameras and exit the application if no camera is found.
        DeviceInfoList_t devices;
        if (0 == pTl->EnumerateDevices(devices))
        {
            cerr << "No camera present!" << endl;
            //pressEnterToExit();
            return ;
        }

        // Create the camera object of the first available camera.
        // The camera object is used to set and get all available
        // camera features.
        Camera_t Camera(pTl->CreateDevice(devices[0]));

        // Open the camera.
        Camera.Open();

        Camera.LineSelector.SetValue(LineSelector_Out1);
        Camera.LineSource.SetValue(LineSource_UserOutput);
        Camera.UserOutputSelector.SetValue(UserOutputSelector_UserOutput1);
        Camera.UserOutputValue.SetValue(value);
        Camera.Close();
    }
}
int basler::disconnect()
{
    // Comment the following two lines to disable waiting on exit.
    cerr << endl << "Press Enter to exit." << endl;
    //while( cin.get() != '\n');

    // Releases all pylon resources.
    PylonTerminate();

}

int basler::start()
{
    // The exit code of the sample application.
    int exitCode = 0;
    try{

        //camera->RegisterConfiguration(new C);
        // Start the grabbing of c_countOfImagesToGrab images.
        // The camera device is parameterized with a default configuration which
        // sets up free-running continuous acquisition.
        camera->StartGrabbing( c_countOfImagesToGrab);

        // This smart pointer will receive the grab result data.
        CGrabResultPtr ptrGrabResult;

        // Camera.StopGrabbing() is called automatically by the RetrieveResult() method
        // when c_countOfImagesToGrab images have been retrieved.
        while ( camera->IsGrabbing())
        {
            // Wait for an image and then retrieve it. A timeout of 5000 ms is used.
            camera->RetrieveResult( 5000, ptrGrabResult, TimeoutHandling_ThrowException);

            // Image grabbed successfully?
            if (ptrGrabResult->GrabSucceeded())
            {
                // Access the image data.
                cout << "SizeX: " << ptrGrabResult->GetWidth() << endl;
                cout << "SizeY: " << ptrGrabResult->GetHeight() << endl;
                const uint8_t *pImageBuffer = (uint8_t *) ptrGrabResult->GetBuffer();

                memcpy(globalImageBuffer,pImageBuffer,WIDTH*HEIGHT);

#ifdef PYLON_WIN_BUILD
                // Display the grabbed image.
                Pylon::DisplayImage(1, ptrGrabResult);
#endif
            }
            else
            {
                cout << "Error: " << ptrGrabResult->GetErrorCode() << " " << ptrGrabResult->GetErrorDescription() << endl;
            }
        }
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
             << e.GetDescription() << endl;
        exitCode = 1;
    }


    return exitCode;
}

int basler::initTrigger()
{

    //    PylonInitialize();
    // Get the transport layer factory.
    CTlFactory& TlFactory = CTlFactory::GetInstance();

    // Create the transport layer object needed to enumerate or
    // create a camera object of type Camera_t::DeviceClass().
    ITransportLayer *pTl = TlFactory.CreateTl(Camera_t::DeviceClass());

    // Exit the application if the specific transport layer is not available.
    if (! pTl)
    {
        cerr << "Failed to create transport layer!" << endl;
        //pressEnterToExit();
        return 1;
    }

    // Get all attached cameras and exit the application if no camera is found.
    DeviceInfoList_t devices;
    if (0 == pTl->EnumerateDevices(devices))
    {
        cerr << "No camera present!" << endl;
        //pressEnterToExit();
        return 1;
    }

    // Create the camera object of the first available camera.
    // The camera object is used to set and get all available
    // camera features.
    Camera = new Camera_t(pTl->CreateDevice(devices[0]));

    // Open the camera.
    Camera->Open();

}

int basler::startTriggerMode()
{
    // Automagically call PylonInitialize and PylonTerminate to ensure that the pylon runtime
    // system is initialized during the lifetime of this object.
    int result = 0;
    try
    {
        // Get the first stream grabber object of the selected camera.
        Camera_t::StreamGrabber_t StreamGrabber(Camera->GetStreamGrabber(0));

        // Open the stream grabber.
        StreamGrabber.Open();

        Camera->AcquisitionMode.SetValue(AcquisitionMode_SingleFrame);
        Camera->TriggerSelector.SetValue( TriggerSelector_AcquisitionStart);
        Camera->TriggerMode.SetValue( TriggerMode_On);
        Camera->TriggerSource.SetValue(TriggerSource_Line1);
        //	Camera.TriggerActivasion.SetValue(TriggerActivation_RisingEdge);
        Camera->AcquisitionFrameCount.SetValue(10);

        // Create an image buffer.
        const size_t ImageSize = (size_t)(Camera->PayloadSize.GetValue());
        uint8_t * const pBuffer = new uint8_t[ ImageSize ];

        // We won't use image buffers greater than ImageSize.
        StreamGrabber.MaxBufferSize.SetValue(ImageSize);

        // We won't queue more than one image buffer at a time.
        StreamGrabber.MaxNumBuffer.SetValue(100);

        // Allocate all resources for grabbing. Critical parameters like image
        // size now must not be changed until FinishGrab() is called.
        StreamGrabber.PrepareGrab();

        // Buffers used for grabbing must be registered at the stream grabber.
        // The registration returns a handle to be used for queuing the buffer.
        const StreamBufferHandle hBuffer =
                StreamGrabber.RegisterBuffer(pBuffer, ImageSize);

        // Put the buffer into the grab queue for grabbing.
        StreamGrabber.QueueBuffer(hBuffer, NULL);

        // Let the camera acquire one  image ( Acquisition mode equals
        // SingleFrame! ).
        Camera->AcquisitionStart.Execute();

        // Wait for the grabbed image with a timeout of 3 seconds.

        bool wait = true;
        while(wait)
        {
            if (StreamGrabber.GetWaitObject().Wait(1000) )
            {
                // Get the grab result from the grabber's result queue.
                GrabResult Result;
                StreamGrabber.RetrieveResult(Result);

                if (Result.Succeeded())
                {
                    // Grabbing was successful, process image.
                    // cout << "Image #" << n << " acquired!" << endl;
                    cout << "Size: " << Result.GetSizeX() << " x " << Result.GetSizeY() << endl;

                    memcpy(globalImageBuffer,(uint8_t *) Result.Buffer(),WIDTH*HEIGHT);
                    //Result.
                    result = 0;
                    wait = false;
                    //StreamGrabber.
                    // Get the pointer to the image buffer.
                    //const uint8_t *pImageBuffer = (uint8_t *) Result.Buffer();
                    //cout << "Gray value of first pixel: " << (uint32_t) pImageBuffer[0]
                    //     << endl << endl;

                    // Reuse the buffer for grabbing the next image.
                    //StreamGrabber.QueueBuffer(Result.Handle(), NULL);
                }
                else
                {
                    if(Result.GetErrorCode() == 3774873620)
                        result = -2;
                    else
                         result = -1;
                    // Error handling
                    cerr << "No image acquired!" << endl;
                    cerr << "Error code : 0x" //<< hex
                         << Result.GetErrorCode() << endl;
                    cerr << "Error description : "
                         << Result.GetErrorDescription() << endl;


                    wait = false;
                }
            }
            else
            {
                if(Main->isTriggeMode && !Main->appClosed)
                {
//                    cout << "t"<<endl;
                    continue;
                }
                // Timeout
                cerr << "Timeout occurred!" << endl;

                // Get the pending buffer back (You are not allowed to deregister
                // buffers when they are still queued).
                StreamGrabber.CancelGrab();

                // Get all buffers back.
                for (GrabResult r; StreamGrabber.RetrieveResult(r););
                Camera->Close();
                result = 1;
                wait = false;
            }
        }

        // Clean up

        // You must deregister the buffers before freeing the memory.
        StreamGrabber.DeregisterBuffer(hBuffer);

        // Free all resources used for grabbing.
        StreamGrabber.FinishGrab();

        // Close stream grabber.
        StreamGrabber.Close();

        // Close camera.
        //        Camera->Close();


        // Free memory of image buffer.
        delete[] pBuffer;
        return result;

    }
    catch (const GenericException &e)
    {
        // Error handling
        cerr << "An exception occurred3" << endl
             << e.GetDescription() << endl;

        result = -1;
    }

    // Quit the application.
    return result;
}

void basler::loadConfig()
{
    const char Filename[] = "NodeMap.pfs";
//    printf("filename: %s\n",Filename);
    // The exit code of the sample application.
    int exitCode = 0;

    // Before using any pylon methods, the pylon runtime must be initialized.
    PylonInitialize();

    try
    {
        // Create an instant camera object with the camera device found first.
        CInstantCamera camera( CTlFactory::GetInstance().CreateFirstDevice());

        // Print the model name of the camera.
        cout << "Using device " << camera.GetDeviceInfo().GetModelName() << endl;

        // Open the camera.
        camera.Open();

        cout << "Saving camera's node map to file..."<< endl;
        // Save the content of the camera's node map into the file.
        //        CFeaturePersistence::Save( Filename, &camera.GetNodeMap() );
        // --------------------------------------------------------------------

        // Just for demonstration, read the content of the file back to the camera's node map with enabled validation.
        cout << "Reading file back to camera's node map..."<< endl;
//        qDebug()<<QDir::currentPath();
        CFeaturePersistence::Load( Filename, &camera.GetNodeMap(), true );

        // Close the camera.
        camera.Close();
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
             << e.GetDescription() << endl;
        exitCode = 1;
    }

    // Comment the following two lines to disable waiting on exit

    // Releases all pylon resources.
    PylonTerminate();

    return ;
}








