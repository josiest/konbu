# Konbu
Konbu is a parsing utility library built on top of yaml-cpp. The motivation
behind it to provide a reasonable way to read data with default values, while
still being able to keep track of any parsing failures that happen along the
way.

## Requirements
- C++23 compiler
- yaml-cpp

As mentioned before, konbu is built on top of yaml-cpp, so you'll need to have
yaml-cpp installed on your system. The interface also plans to use
`std::expected`, which is a library feature from the C++23 standard. So you'll
also need a C++ compiler that supports this standard or higher.

## Installation
So far konbu is a single-include header library, so feel free to just copy it
directly into your project. Alternatively, if you want to use cmake, you can
also run the following commands

```cpp

git clone https://github.com/josiest/konbu.git
cd konbu
mkdir build
cd build
cmake ..
cmake --install . --prefix <path/to/your/project>
```

You can then add this to your project's `CMakeLists.txt`

```CMake
find_package(konbu 0.1.0 EXACT REQUIRED PATHS lib/cmake)
target_link_libraries(<your-project> INTERFACE konbu::konbu)
```
Note that this example uses the `--prefix` option for installing via cmake, and
that `find_package` uses the exact version number. Because konbu is in its early
stages of development, its interface is fairly volatile. So I'd highly recommend
that you install it on a per-project basis with a specific version. This way, if
breaking changes are introduced to the interface, your project isn't broken
along with it.

## Usage
So far this library is fairly small in scope. There are only a few template
concepts and utility functions in the library. However, when using konbu, it's
implied that you'll be writing `read` function overloads with a specific
interface. This looks like

```cpp
namespace konbu {
template<std::ranges::output_range<YAML::Exception> error_output>
void read(YAML::Node const & config, your_class & value, error_output & errors)
{
    // ... parse your_class ...
}
}
```

This interface has some expectations:
- `error_output` is an allocator-aware container
- any errors encountered while parsing should be written to `errors` via an
  `insert_iterator`
- `your_class` should be default-constructable so that if any errors happen
  along the way, the value that already exists will be used.

You can then use the function somewhat like this:
```cpp
void print_error(std::string const & error)
{
    std::cout << error << "\n\n";
}

int main()
{
    auto const config = YAML::LoadFile("path/to/your/asset.yaml");
    std::vector<YAML::Exception> errors;
    
    your_class value;
    konbu::read(config, value, errors);
    
    ranges::for_each(errors | views::transform(&YAML::Exception::what),
                     print_error);
}
```

## Examples
For more details and examples of how to use te library, see
`examples/sketch.cpp` for a data interface for a prototype UI library