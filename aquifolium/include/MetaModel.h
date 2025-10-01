/*
 * OpenHydroQual - Environmental Modeling Platform
 * Copyright (C) 2025 Arash Massoudieh
 *
 * This file is part of OpenHydroQual.
 *
 * OpenHydroQual is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * If you use this file in a commercial product, you must purchase a
 * commercial license. Contact arash.massoudieh@enviroinformatics.co for details.
 */

#ifndef METAMODEL_H
#define METAMODEL_H

#include "QuanSet.h"
#include <map>
#include <iostream>
#include <fstream>
#include <vector>

/**
 * @class MetaModel
 * @brief A container for managing collections of QuanSet objects indexed by type name
 *
 * MetaModel extends std::map to provide specialized functionality for storing and
 * managing QuanSet objects. Each QuanSet is associated with a unique string key
 * representing its type. The class provides JSON serialization/deserialization
 * capabilities and maintains solution variable ordering.
 *
 * @note This class inherits from std::map and should NOT be used polymorphically
 *       (i.e., never delete through a std::map pointer). This is safe as long as
 *       MetaModel objects are used directly and not through base class pointers.
 *
 * @warning The class uses a static empty QuanSet for error returns in at(), which
 *          is not thread-safe. Consider exception-based error handling for
 *          multi-threaded applications.
 *
 * Example usage:
 * @code
 *   MetaModel model;
 *   model.GetFromJsonFile("config.json");
 *
 *   QuanSet* qs = model.GetItem("Block");
 *   if (qs) {
 *       // Use the QuanSet
 *   }
 * @endcode
 */
class MetaModel: public map<string, QuanSet>
{
public:
    /**
         * @brief Default constructor
         *
         * Constructs an empty MetaModel with no QuanSet objects.
         */
    MetaModel();

    /**
         * @brief Destructor
         *
         * Automatically destroys all contained QuanSet objects and frees resources.
         * No explicit cleanup is needed as all members are self-managing.
         */
    virtual ~MetaModel();

    /**
         * @brief Copy constructor
         * @param other The MetaModel object to copy from
         *
         * Creates a deep copy of the MetaModel, including all QuanSet objects,
         * solution variable order, and error states.
         */
    MetaModel(const MetaModel& other);

    /**
         * @brief Copy assignment operator
         * @param other The MetaModel object to copy from
         * @return Reference to this object
         *
         * Performs a deep copy assignment with self-assignment check.
         */
    MetaModel& operator=(const MetaModel& other);

    /**
         * @brief Appends a new QuanSet to the model
         * @param s The type name/key for the QuanSet
         * @param q The QuanSet object to append
         * @return true if successfully added, false if key already exists
         *
         * Adds a new QuanSet with the specified type name. If a QuanSet with
         * the same name already exists, the operation fails and an error is logged.
         *
         * @note On failure, check GetLastError() for details
         */
    bool Append(const string &s, const QuanSet &q);

    /**
         * @brief Counts the number of QuanSet objects with the specified type
         * @param s The type name to search for
         * @return The count (0 or 1, since keys are unique)
         *
         * Returns 1 if a QuanSet with the specified type exists, 0 otherwise.
         */
    unsigned long Count(const string &s) const {return count(s);}

    /**
         * @brief Accesses a QuanSet by type name (non-const version)
         * @param type The type name of the QuanSet to retrieve
         * @return Reference to the QuanSet, or a static empty QuanSet if not found
         *
         * @warning Returns a static empty QuanSet on error, which is shared across
         *          all error cases. Check Count() first or use GetItem() for pointer-based
         *          null checking instead.
         * @note Sets last_error if the type is not found
         */
    QuanSet& at(const string &type);

    /**
         * @brief Accesses a QuanSet by type name (const version)
         * @param type The type name of the QuanSet to retrieve
         * @return Const reference to the QuanSet, or a static empty QuanSet if not found
         *
         * This version is called when the MetaModel object is const.
         *
         * @warning Returns a static empty QuanSet on error, which is shared across
         *          all error cases. Check Count() first for safer access.
         * @note Sets last_error if the type is not found
         */
    const QuanSet& at(const string &type) const;

    /**
         * @brief Gets a pointer to the underlying map
         * @return Pointer to this object cast as map<string, QuanSet>*
         *
         * Provides direct access to the underlying map interface.
         */
    map<string, QuanSet> *GetMetaModel() {return this;}

    /**
         * @brief Retrieves a pointer to a QuanSet by type name (non-const version)
         * @param typ The type name of the QuanSet to retrieve
         * @return Pointer to the QuanSet, or nullptr if not found
         *
         * Safer alternative to at() as it returns nullptr on failure instead of
         * a static empty object.
         *
         * @note Sets last_error if the type is not found
         */
    QuanSet* operator[] (const string &typ);

    /**
         * @brief Retrieves a pointer to a QuanSet by type name (const version)
         * @param typ The type name of the QuanSet to retrieve
         * @return Const pointer to the QuanSet, or nullptr if not found
         *
         * This version is called when the MetaModel object is const.
         *
         * @note Sets last_error if the type is not found
         */
    const QuanSet* operator[] (const string &typ) const;

    /**
         * @brief Retrieves a pointer to a QuanSet by type name (non-const version)
         * @param typ The type name of the QuanSet to retrieve
         * @return Pointer to the QuanSet, or nullptr if not found
         *
         * Functionally identical to operator[] but with a more explicit name.
         * Prefer this over operator[] for clarity.
         *
         * @note Sets last_error if the type is not found
         */
    QuanSet* GetItem(const string &typ);

    /**
         * @brief Retrieves a pointer to a QuanSet by type name (const version)
         * @param typ The type name of the QuanSet to retrieve
         * @return Const pointer to the QuanSet, or nullptr if not found
         *
         * This version is called when the MetaModel object is const.
         *
         * @note Sets last_error if the type is not found
         */
    const QuanSet* GetItem(const string &typ) const;

    /**
         * @brief Loads MetaModel configuration from a JSON file
         * @param filename Path to the JSON file to load
         * @return true if successful, false on error
         *
         * Clears the current MetaModel and loads a new configuration from the
         * specified JSON file. The file should contain object types as top-level
         * keys, with each containing QuanSet definitions.
         *
         * Special handling for "solutionorder" key: if present, populates the
         * solvevariableorder vector.
         *
         * @note On failure, check GetLastError() for details
         * @see AppendFromJsonFile() for adding to existing model without clearing
         */
    bool GetFromJsonFile(const string &filename);

    /**
         * @brief Appends MetaModel configuration from a JSON file
         * @param filename Path to the JSON file to load
         * @return true if successful, false on error
         *
         * Loads QuanSet objects from a JSON file and appends them to the existing
         * MetaModel without clearing current contents. If duplicate type names exist,
         * they will not be added (see Append() behavior).
         *
         * @note On failure, check GetLastError() for details
         * @see GetFromJsonFile() for clearing and loading
         */
    bool AppendFromJsonFile(const string& filename);

    /**
         * @brief Clears all data from the MetaModel
         *
         * Removes all QuanSet objects, clears the solution variable order,
         * and clears the error list. Does not clear last_error.
         */
    void Clear();

    /**
         * @brief Converts the MetaModel to a string representation
         * @param _tabs Number of tab indentations for formatting (default: 0)
         * @return String representation of the MetaModel
         *
         * Generates a human-readable string representation of the entire MetaModel,
         * including all QuanSet objects with proper indentation.
         */
    string ToString(int _tabs=0) const;

    /**
         * @brief Ordered list of variables for solution sequencing
         *
         * Contains the order in which variables should be solved. Populated
         * from the "solutionorder" array in JSON configuration files.
         */
    vector<string> solvevariableorder;

    /**
         * @brief Retrieves the last error message
         * @return The last error message as a string
         *
         * Returns the most recent error message set by failed operations.
         * Empty string if no errors have occurred.
         */
    string GetLastError() const {return last_error; }

    /**
         * @brief Renames a constituent across all QuanSet objects
         * @param oldname The current name of the constituent (before colon)
         * @param newname The new name for the constituent
         *
         * Systematically renames all properties that match the pattern "oldname:*"
         * to "newname:*" across all QuanSet objects in the model. Also updates
         * the descriptions accordingly.
         *
         * Example: RenameConstituent("Temperature", "Temp") would rename
         * "Temperature:inlet" to "Temp:inlet", "Temperature:outlet" to "Temp:outlet", etc.
         */
    void RenameConstituent(const string &oldname, const string &newname);

protected:

private:
    /**
         * @brief Stores the most recent error message
         *
         * Mutable to allow error tracking even in const methods without
         * affecting the logical const-ness of the object.
         */
    mutable string last_error;

    /**
         * @brief Historical log of all error messages
         *
         * Accumulates error messages over the lifetime of the object.
         * Cleared by Clear() but not by individual operations.
         */
    vector<string> errors;
};

#endif // METAMODEL_H
