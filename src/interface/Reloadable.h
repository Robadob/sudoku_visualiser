#ifndef SRC_INTERFACE_RELOADABLE_H_
#define SRC_INTERFACE_RELOADABLE_H_

/**
 * Interface for classes that can be reloaded
 * @see ShaderCore
 * @see EntityCore
 * @see TextureCore
 */
class Reloadable {
 public:
    /**
     * Public virtual destructor allows Pointers to this type to destroy subclasses correctly
     */
    virtual ~Reloadable() { }
    /**
     * Calling this reloads the subclass
     */
    virtual void reload() = 0;
};

#endif  // SRC_INTERFACE_RELOADABLE_H_
