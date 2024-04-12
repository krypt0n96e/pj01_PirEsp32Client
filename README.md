<a name="readme-top"></a>
<!-- PROJECT NAME -->
# [Client on ESP32 for Database collection system serving smart PIR sensors][project-esp-url]


<!-- TABLE OF CONTENTS -->
## Table of Contents
- [Client on ESP32 for Database collection system serving smart PIR sensors](#client-on-esp32-for-database-collection-system-serving-smart-pir-sensors)
  - [Table of Contents](#table-of-contents)
  - [About The Project](#about-the-project)
    - [Built With](#built-with)
  - [Getting Started](#getting-started)
    - [Prerequisites](#prerequisites)
    - [Deployment](#deployment)
  - [Usage](#usage)
  - [Acknowledgments](#acknowledgments)




<!-- ABOUT THE PROJECT -->
## About The Project

![project_demo](readme_include/project_demo.png)

Project objectives:
* Build a system to collect signals from PIR sensors and cameras to serve the model training process. ​
* Store data and process the obtained data.​
* There is an interface for users to perform tasks with data.

<p align="right">(<a href="#readme-top">back to top</a>)</p>



### Built With

* [![C][C-logo]][C-url]
* [![Espressif][Espressif-logo]][Espressif-url]
<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

This is an example of how you may give instructions on setting up your project locally.

### Prerequisites

First, you need to install Python and set up the environment.

* Install Python:
  * For MacOS/Linux:
    ```bash
    $ sudo apt-get update
    $ sudo apt-get install python3
    ```
  * For Windows:
    * you can [download Python][Python-url] and install from python website or using choco
    ```cmd
    > choco install python
    ```
* Install virtual environment:
  * Create a project folder and a `.venv` folder within:
    * For MacOS/Linux:
      ```bash
      $ mkdir myproject
      $ cd myproject
      $ python3 -m venv .venv
      ```
    * For Windows:
      ```cmd
      > mkdir myproject
      > cd myproject
      > python -m venv .venv
      ```
* Activate the environment:
  * Before you work on your project, activate the corresponding environment:
    * For MacOS/Linux:
      ```bash
      $ source .venv/bin/activate
      ```
    * For Windows:
      ```cmd
      > .venv\Scripts\activate
      ```
* Install libraries:
  * OpenCV, Flask, SQLAlchemy:
    ```bash
    $ pip install Flask Flask-SQLAlchemy opencv-python
    ```

 
### Deployment

* Deploy on the local network using Flask deploy tool:
  * Run in the terminal in the project folder:
    * MacOS/Linux:
      ```bash
      $ python3 main.py
      ```
    * Windows:
      ```cmd
      > python main.py
      ```

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- USAGE EXAMPLES -->
## Usage

After deploying the server, you can access the homepage via your_server_ip:8888 to interact with the server and database. You can see the features as shown below

![homepage_usage](readme_include/homepage_usage.png)

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- CONTACT -->
<!-- ## Contact


<p align="right">(<a href="#readme-top">back to top</a>)</p>
 -->


<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

* Project on server side: 
  [![github][github-logo]][project-server-url]

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- MARKDOWN LINKS & IMAGES -->
[project-server-url]:https://github.com/krypt0n96e/pj01_PirFlaskServer
[project-esp-url]:https://github.com/krypt0n96e/pj01_PirEsp32Client
[github-logo]:https://img.shields.io/badge/github-121013?style=for-the-badge&logo=github&logoColor=white
[C-logo]:https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white
[C-url]: https://www.w3schools.com/c/index.php
[Espressif-logo]:https://img.shields.io/badge/espressif-E7352C.svg?style=for-the-badge&logo=espressif&logoColor=white
[Espressif-url]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/
