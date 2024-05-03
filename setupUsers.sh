#!/bin/bash
useradd SalesManager
useradd DistributionManager
useradd ManufacturingManager

groupadd Sales
groupadd Distribution
groupadd Manufacturing

adduser SalesManager Sales
adduser DistributionManager Distribution
adduser ManufacturingManager Manufacturing