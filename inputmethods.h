#ifndef INPUTMETHODS_H
#define INPUTMETHODS_H

#include <QString>

class InputMethod
{
    public:
        explicit InputMethod() {}
        virtual ~InputMethod() {}
        virtual void run() = 0;
        virtual void kill() = 0;
        virtual void saveEnvSettings() const = 0;
        virtual bool getVersion( QString& version ) const = 0;
};

class FcitxInputMethod : public InputMethod
{
    public:
        static FcitxInputMethod* self();
        virtual ~FcitxInputMethod();
        virtual void run();
        virtual void kill();
        virtual void saveEnvSettings() const;
        virtual bool getVersion( QString& version ) const;
    private:
        explicit FcitxInputMethod();
        static FcitxInputMethod* m_self;
};

class IBusInputMethod : public InputMethod
{
    public:
        static IBusInputMethod* self();
        virtual ~IBusInputMethod();
        virtual void run();
        virtual void kill();
        virtual void saveEnvSettings() const;
        virtual bool getVersion( QString& version ) const;
    private:
        explicit IBusInputMethod();
        static IBusInputMethod* m_self;
};

class SCIMInputMethod : public InputMethod
{
    public:
        static SCIMInputMethod* self();
        virtual ~SCIMInputMethod();
        virtual void run();
        virtual void kill();
        virtual void saveEnvSettings() const;
        virtual bool getVersion( QString& version ) const;
    private:
        explicit SCIMInputMethod();
        static SCIMInputMethod* m_self;
};

#endif // INPUTMETHODS_H
