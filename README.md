# XMI-VLE Converter
This tool aims to facilitate the transformation of UML diagrams (Sequence Diagrams) to DEVS models to be used in [Virtual Library Environment](http://www.vle-project.org/).

## Requirements
* boost
* c++ compiler
---
## Manual
### Synopsis
./xmi2vle [SOURCE_FILE] [DESTINATION_FILE]

### Description
**xmi2vle** reads an input XMI file of a sequence diagram, translate them into another XMI format called VPZ, and generate initial CPP files to be used in a [VLE](https://github.com/vle-forge/vle) Project. The original XMI must be that of a sequence diagram. The preferred UML tool to generate XMI is **Modelio** which is free and open-source. Modelio can be downloaded at [this link](https://modelio.org). For best compatibility, use Modelio to export diagrams in XMI format following **OMG UML2.4.1 standards** and not **EMF UML3.0.0** of Eclipse Modeling Framework.

In Modelio, refer to the follwing rules when creating a sequence diagram:

* **xmi2vle** only reads a single XMI file at at time, therefore the generated XMI should contain the main model followed by its coupled models. In Modelio, multiple sequence diagrams can be created under the same project directory. **xmi2vle** takes the first diagram as the main model.

* Model names should be under the form **MODEL_TYPE:MODEL_NAME** of which the MODEL_TYPE can be Atomic or Coupled. Examples: *Atomic:Host*, *Coupled:Server*, etc.

* State and *ta* (time advance of the state) should be a plain text **Note** in which the message follows the format **STATE_NAME/time=STATE_TIME**. Example: *Execute/time=12* designates a state named Execute with the time advance of 12 units of time.
