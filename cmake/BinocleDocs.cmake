# first we can indicate the documentation build as an option and set it to ON by default
option(BUILD_DOC "Build documentation" OFF)

if (BUILD_DOC)

    # check if Doxygen is installed
    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # set input and output files
        set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
        set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

        # request to configure the file
        configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
    else (DOXYGEN_FOUND)
      message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)

    find_package(Sphinx REQUIRED)

    if (SPHINX_FOUND)
        set(SPHINX_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/docs)
        set(SPHINX_BUILD ${CMAKE_CURRENT_BINARY_DIR}/doc_sphinx)
        set(SPHINX_INDEX_FILE ${SPHINX_BUILD}/index.html)

        message("Sphinx build started")
        add_custom_command(OUTPUT ${SPHINX_INDEX_FILE}
                COMMAND
                ${SPHINX_EXECUTABLE} -b html
                # Tell Breathe where to find the Doxygen output
                -Dbreathe_projects.Binocle=${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/xml
                ${SPHINX_SOURCE} ${SPHINX_BUILD}
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                DEPENDS
                # Other docs files you want to track should go here (or in some variable)
                ${CMAKE_CURRENT_SOURCE_DIR}/docs/index.rst
                ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/xml/index.xml
                MAIN_DEPENDENCY ${SPHINX_SOURCE}/conf.py
                COMMENT "Generating documentation with Sphinx")
        add_custom_target(doc_sphinx ALL DEPENDS ${SPHINX_INDEX_FILE})
    else (SPHINX_FOUND)
        message("Sphinx need to be installed to generate the sphinx documentation")
    endif(SPHINX_FOUND)

endif(BUILD_DOC)