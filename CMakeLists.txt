cmake_minimum_required(VERSION 3.16)

project(SeekCytometer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_PREFIX_PATH "D:/Qt/6.8.0/llvm-mingw_64")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets LinguistTools Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Sql Network Charts StateMachine)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)





# # Only enable debug in DEBUG BUILD
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     target_compile_definitions(my_app
#         PRIVATE
#             ENABLE_DEBUG=1
#     )
# endif()


set(TS_FILES SeekCytometer_zh_CN.ts)

set(PROJECT_SOURCES
        main.cpp
        MainWindow.cpp
        MainWindow.h
        ${TS_FILES}
)

include_directories(. network datamodel database widgets dialogs delegate data_visualization data_manage test)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)

    qt_add_executable(SeekCytometer
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}


        database/DatabaseManager.h database/DatabaseManager.cpp
        database/User.h database/User.cpp
        database/Experiment.h database/Experiment.cpp
        database/Specimen.h database/Specimen.cpp
        database/Tube.h database/Tube.cpp
        widgets/ExperimentsBrowser.h widgets/ExperimentsBrowser.cpp
        widgets/CytometerSettingsWidget.h widgets/CytometerSettingsWidget.cpp
        widgets/DetectorSettingsWidget.h widgets/DetectorSettingsWidget.cpp
        database/DetectorSettings.h database/DetectorSettings.cpp
        database/Detector.h database/Detector.cpp
        database/CytometerSettings.h database/CytometerSettings.cpp
        datamodel/DetectorSettingsModel.h datamodel/DetectorSettingsModel.cpp
        datamodel/DetectorModel.h datamodel/DetectorModel.cpp
        database/BaseDAO.h database/BaseDAO.cpp
        database/DetectorsDAO.h database/DetectorsDAO.cpp
        database/DetectorSettingsDAO.h database/DetectorSettingsDAO.cpp
        database/TubesDAO.h database/TubesDAO.cpp
        database/SpecimensDAO.h database/SpecimensDAO.cpp
        database/ExperimentsDAO.h database/ExperimentsDAO.cpp
        database/UsersDAO.h database/UsersDAO.cpp
        database/CytometerSettingsDAO.h database/CytometerSettingsDAO.cpp
        database/BrowserDataDAO.h database/BrowserDataDAO.cpp
        database/BrowserData.h database/BrowserData.cpp
        datamodel/BrowserDataModel.h datamodel/BrowserDataModel.cpp
        dialogs/AddNewItemDialog.h dialogs/AddNewItemDialog.cpp
        dialogs/AddNewDetectorDialog.h dialogs/AddNewDetectorDialog.cpp
        delegate/CheckBoxDelegate.h delegate/CheckBoxDelegate.cpp
        dialogs/LoginDialog.h dialogs/LoginDialog.cpp

        widgets/CustomStatusBar.cpp widgets/CustomStatusBar.h
        widgets/StatusIndicator.cpp widgets/StatusIndicator.h

        widgets/MenuBarManager.h widgets/MenuBarManager.cpp
        database/NodeTypeHelper.h database/NodeTypeHelper.cpp
        widgets/DataAcquisitionWidget.h widgets/DataAcquisitionWidget.cpp
        database/WorkSheet.h database/WorkSheet.cpp
        database/WorkSheetsDAO.h database/WorkSheetsDAO.cpp
        widgets/WorkSheetWidget.h widgets/WorkSheetWidget.cpp
        data_visualization/WorkSheetView.h data_visualization/WorkSheetView.cpp
        data_visualization/WorkSheetScene.h data_visualization/WorkSheetScene.cpp
        data_visualization/GateItem.h data_visualization/GateItem.cpp
        database/Gate.h database/Gate.cpp
        data_visualization/ScatterPlot.h data_visualization/ScatterPlot.cpp
        data_visualization/HistogramPlot.h data_visualization/HistogramPlot.cpp
        dialogs/AddNewPlotDialog.h dialogs/AddNewPlotDialog.cpp
        widgets/CytometerGeneralInfo.h widgets/CytometerGeneralInfo.cpp
        data_visualization/CustomAxis.h data_visualization/CustomAxis.cpp
        database/Plot.h database/Plot.cpp
        database/PlotsDAO.h database/PlotsDAO.cpp
        database/GatesDAO.h database/GatesDAO.cpp
        test/TestDataGenerator.h test/TestDataGenerator.cpp
        data_visualization/PlotBase.h data_visualization/PlotBase.cpp
        database/MeasurementTypeHelper.h database/MeasurementTypeHelper.cpp
        data_manage/RingBuffer.h
        data_manage/ChartBuffer.h

        data_manage/DataManager.h data_manage/DataManager.cpp
        CytometerController.h CytometerController.cpp
        data_visualization/RectangleGateItem.h data_visualization/RectangleGateItem.cpp
        data_visualization/PolygonGateItem.h data_visualization/PolygonGateItem.cpp
        data_visualization/EllipseGateItem.h data_visualization/EllipseGateItem.cpp
        data_visualization/IntervalGateItem.h data_visualization/IntervalGateItem.cpp
        data_visualization/QuadrantGateItem.h data_visualization/QuadrantGateItem.cpp
        data_visualization/GateItemFactory.h data_visualization/GateItemFactory.cpp

        network/UdpCommFrame.h network/UdpCommFrame.cpp
        network/UdpCommClient.h network/UdpCommClient.cpp
        dialogs/UserManageDialog.h dialogs/UserManageDialog.cpp
        widgets/SortingWidget.h widgets/SortingWidget.cpp
        data_visualization/WaveformView.cpp data_visualization/WaveformView.h
        data_visualization/Waveform.cpp data_visualization/Waveform.h
        widgets/WaveformWidget.cpp widgets/WaveformWidget.h
        data_manage/EventDataManager.h data_manage/EventDataManager.cpp
        data_manage/EventData.h data_manage/EventData.cpp
        datamodel/GatesModel.h datamodel/GatesModel.cpp
        delegate/TubeButtonDelegate.h delegate/TubeButtonDelegate.cpp
        widgets/BrowserView.h widgets/BrowserView.cpp
        datamodel/TubeSelectModel.h datamodel/TubeSelectModel.cpp
        widgets/SpeedMeasureWidget.h widgets/SpeedMeasureWidget.cpp
        widgets/SampleChipWidget.h widgets/SampleChipWidget.cpp
        widgets/MicroFluidicWidget.h widgets/MicroFluidicWidget.cpp
        widgets/OpticsControlWidget.h widgets/OpticsControlWidget.cpp
        resource.qrc
        app_icon.rc
        data_manage/Logger.h data_manage/Logger.cpp
        data_visualization/AxisLockButtonItem.h data_visualization/AxisLockButtonItem.cpp
        data_visualization/SaveImageButtonItem.h data_visualization/SaveImageButtonItem.cpp
        data_visualization/AxisAutoAdjustButton.h data_visualization/AxisAutoAdjustButton.cpp
        data_visualization/AxiTypeSwitchButton.h data_visualization/AxiTypeSwitchButton.cpp


    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SeekCytometer APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(SeekCytometer SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SeekCytometer
            ${PROJECT_SOURCES}
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(SeekCytometer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Sql Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::StateMachine)
target_link_libraries(SeekCytometer PRIVATE Qt6::Core)
target_link_libraries(SeekCytometer PRIVATE Qt6::Core)
target_link_libraries(SeekCytometer PRIVATE Qt6::Core)
target_link_libraries(SeekCytometer PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(SeekCytometer PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(SeekCytometer PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(SeekCytometer PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(SeekCytometer PRIVATE Qt${QT_VERSION_MAJOR}::Core)

# FOR DEBUG
target_compile_definitions(SeekCytometer PRIVATE ENABLE_DEBUG=0)


# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SeekCytometer)
endif()
set_target_properties(SeekCytometer PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SeekCytometer
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SeekCytometer)
endif()
