# Simple Python test file for the parser

def factorial(n):
    if n <= 1:
        return 1
    else:
        return n * factorial(n - 1)

# Test the factorial function
result = factorial(5)
print("Factorial of 5 is:", result)

# Simple class definition
class Person:
    def __init__(self, name, age):
        self.name = name
        self.age = age
    
    def greet(self):
        return "Hello, my name is " + self.name

# Create an instance of Person
john = Person("John", 30)
greeting = john.greet()
print(greeting)

# Simple for loop
for i in range(5):
    print(i)

# Simple while loop
count = 0
while count < 5:
    print(count)
    count += 1
