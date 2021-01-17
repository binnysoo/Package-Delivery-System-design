# Package-Delivery-System-design
> 2020 Spring Database Systems (CSE4110)
> Project1. E-R design and Relational Schema design

### Goal 
The goal of this project is to provide a realistic experience in the conceptual design, logical design and maintenance of a small relational database.

### Requirements
1. E-R model
2. Relational Schema Diagram

### Queries
* Assume truck 1721 is destroyed in a crash. Find all customers who had a package on the truck at the time of the crash. Find all recipients who had a package on that truck at the time of the crash. Find the last successful delivery by that truck prior to the crash.
* Find the customer who has shipped the most packages in the past year.
* Find the customer who has spent the most money on shipping in the past year.
* Find those packages that were not delivered within the promised time.
* Generate the bill for each customer for the past month. Consider creating several types of bills.
  - A simple bill: customer, address, and amount owed.
  - A bill listing charges by type of service.
  - An itemize billing listing each individual shipment and the charges for it.
  Customers like their bills to be readable. While the client will accept a relational table coming from Oracle as the bill, it would be “nice” to have a good-looking bill.
