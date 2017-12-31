//
//  VoodooI2CMultitouchHIDEventDriver.hpp
//  VoodooI2CHID
//
//  Created by Alexandre on 13/09/2017.
//  Copyright © 2017 Alexandre Daoud. All rights reserved.
//

#ifndef VoodooI2CMultitouchHIDEventDriver_hpp
#define VoodooI2CMultitouchHIDEventDriver_hpp

// hack to prevent IOHIDEventDriver from loading when
// we include IOHIDEventService

#define _IOKIT_HID_IOHIDEVENTDRIVER_H

#include <libkern/OSBase.h>

#include <IOKit/IOLib.h>
#include <IOKit/IOKitKeys.h>
#include <IOKit/IOService.h>

#include <IOKit/hid/IOHIDEvent.h>
#include <IOKit/hidevent/IOHIDEventService.h>
#include <IOKit/hid/IOHIDEventTypes.h>
#include <IOKit/hidsystem/IOHIDTypes.h>
#include <IOKit/hid/IOHIDPrivateKeys.h>
#include <IOKit/hid/IOHIDUsageTables.h>
#include <IOKit/hid/IOHIDDevice.h>


#include "VoodooI2CHIDDevice.hpp"
#include "../../../Multitouch Support/VoodooI2CDigitiserStylus.hpp"
#include "../../../Multitouch Support/VoodooI2CMultitouchInterface.hpp"
#include "../../../Multitouch Support/MultitouchHelpers.hpp"

#include "../../../Dependencies/helpers.hpp"

#define kHIDUsage_Dig_Confidence kHIDUsage_Dig_TouchValid

struct __attribute__((__packed__)) {
    uint8_t ReportID;
    uint8_t contact_count;
} TestReport;

/* Implements an HID Event Driver for HID devices that expose a digitiser usage page.
 *
 * The members of this class are responsible for parsing, processing and interpreting digitiser-related HID objects.
 */

class VoodooI2CMultitouchHIDEventDriver : public IOHIDEventService {
  OSDeclareDefaultStructors(VoodooI2CMultitouchHIDEventDriver);

 public:
    IOHIDElement* input_mode_element;
    IOHIDElement* contact_count_element;
    IOHIDElement* contact_count_max_element;
    struct {
        OSArray*           transducers;
        bool               native;
    } digitiser;

    /* Calibrates an HID element
     * @element The element to be calibrated
     * @removalPercentage The percentage by which the element is calibrated
     */

    void calibrateJustifiedPreferredStateElement(IOHIDElement * element, SInt32 removalPercentage);

    /* Notification that a provider has been terminated, sent after recursing up the stack, in leaf-to-root order.
     * @options The terminated provider of this object.
     * @defer If there is pending I/O that requires this object to persist, and the provider is not opened by this object set defer to true and call the IOService::didTerminate() implementation when the I/O completes. Otherwise, leave defer set to its default value of false.
     *
     * @return *true*
     */

    bool didTerminate(IOService* provider, IOOptionBits options, bool* defer);

    /* Called during the interrupt routine to interate over transducers
     * @timestamp The timestamp of the interrupt report
     * @report_id The report ID of the interrupt report
     */

    void handleDigitizerReport(AbsoluteTime timestamp, UInt32 report_id);

    /* Called during the interrupt routine to set transducer values
     * @transducer The transducer to be updated
     * @timestamp The timestamp of the interrupt report
     * @report_id The report ID of the interrupt report
     */

    void handleDigitizerTransducerReport(VoodooI2CDigitiserTransducer* transducer, AbsoluteTime timestamp, UInt32 report_id);

    /* Called during the interrupt routine to handle an interrupt report
     * @timestamp The timestamp of the interrupt report
     * @report A buffer containing the report data
     * @report_type The type of HID report
     * @report_id The report ID of the interrupt report
     */

    virtual void handleInterruptReport(AbsoluteTime timestamp, IOMemoryDescriptor* report, IOHIDReportType report_type, UInt32 report_id);

    /* Called during the start routine to set up the HID Event Driver
     * @provider The <IOHIDInterface> object which we have matched against.
     *
     * This function is reponsible for opening a client connection with the <IOHIDInterface> provider and for publishing
     * a multitouch interface into the IOService plane.
     *
     * @return *true* on successful start, *false* otherwise
     */

    bool handleStart(IOService* provider);

    /* Parses a digitiser usage page element
     * @element The element to parse
     *
     * This function is reponsible for examining the child elements of a digitser elements to determine the
     * capabilities of the digitiser.
     *
     * @return *kIOReturnSuccess* on successful parse, *kIOReturnNotFound* if the digitizer element is not supported
     */

    IOReturn parseDigitizerElement(IOHIDElement* element);

    /* Parses a digitiser transducer element
     * @element The element to parse
     * @parent The parent digitiser
     *
     * This function is reponsible for examining the transducers of a digitiser to determine the capabilities of the transducer.
     *
     * @return *kIOReturnSuccess* on successful parse, *kIOReturnDeviceError*, *kIOReturnError* or *kIOReturnNoDevice* if the transducer element is not supported
     */

    IOReturn parseDigitizerTransducerElement(IOHIDElement* element, IOHIDElement* parent);

    /* Parses all matched elements
     *
     * @return *kIOReturnSuccess* on successful parse, *kIOReturnNotFound* if the matched elements are not supported, *kIOReturnError* otherwise
     */

    IOReturn parseElements();

    /* Postprocessing of digitizer elements
     *
     * This function is mostly copied from Apple's own HID Event Driver code. It is responsible for cleaning up malformed report descriptors as well as setting some miscellaneous properties.
     */

    void processDigitizerElements();

    /* Publishes a <VoodooI2CMultitouchInterface> into the IOService plane
     *
     * @return *kIOReturnSuccess* on successful publish, *kIOReturnError* otherwise.
     */

    IOReturn publishMultitouchInterface();

    /* Sets a button state to the given value
     * @state The button state to be set
     * @bit The bit in which to write the value
     * @value The value to be written
     * @timestamp The timestamp pertaining to the value
     */

    static inline void setButtonState(DigitiserTransducerButtonState* state, UInt32 bit, UInt32 value, AbsoluteTime timestamp);

    /* Publishes some miscellaneous properties to the IOService plane
     */

    void setDigitizerProperties();

    /* Called by the OS in order to notify the driver that the device should change power state
     * @whichState The power state the device is expected to enter represented by either
     *  *kIOPMPowerOn* or *kIOPMPowerOff*
     * @whatDevice The power management policy maker
     *
     * This function exists to be overriden by inherited classes should they need it.
     *
     * @return *kIOPMAckImplied* on succesful state change, *kIOReturnError* otherwise
     */

    virtual IOReturn setPowerState(unsigned long whichState, IOService* whatDevice);

    /* Called during the stop routine to terminate the HID Event Driver
     * @provider The <IOHIDInterface> object which we have matched against.
     *
     * This function is reponsible for releasing the resources allocated in <start>
     */

    void handleStop(IOService* provider);

    /* Implemented to set a certain property
     * @provider The <IOHIDInterface> object which we have matched against.
     */

    bool start(IOService* provider);
 protected:
    bool awake = true;
    IOHIDInterface* hid_interface;
 private:
    SInt32 absolute_axis_removal_percentage = 15;
    VoodooI2CMultitouchInterface* multitouch_interface;
    OSArray* supported_elements;
    
    VoodooI2CHIDDevice* hid_device;
};


#endif /* VoodooI2CMultitouchHIDEventDriver_hpp */