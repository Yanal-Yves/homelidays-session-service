using System;
using System.Reflection;

namespace Homelidays.Web.SessionService.Tests
{
    /// <summary>
    /// A helper class that eases reflection operations.
    /// voici Les méthodes à implémenter au fur et à mesure des besoins:
    ///     - internal static object CallNonPublicStaticMethod(string className, string methodName)
    ///     - internal static object InstanciateNonPublicClass(string className)
    /// </summary>
    internal static class ReflectionUtility
    {
        /// <summary>
        /// Call a non public method of an object
        /// </summary>
        /// <param name="objectWithNonPublic">Object whose method to call is non public.</param>
        /// <param name="methodName">Name of the method to call.</param>
        /// <param name="parameters">Table of parameters to pass to the method.</param>
        /// <returns>The object returned by the private method to call.</returns>
        internal static object CallNonPublicMethod(object objectWithNonPublic, string methodName, object[] parameters)
        {
            Type type = objectWithNonPublic.GetType();
            MethodInfo method_info = type.GetMethod(methodName, BindingFlags.Instance | BindingFlags.NonPublic);
            var return_object = method_info.Invoke(objectWithNonPublic, parameters);
         
            return return_object;
        }

        /// <summary>
        /// Call a non public property of an object.
        /// </summary>
        /// <param name="instance">Object whose method to call is private.</param>
        /// <param name="propertyName">Name of the property to call.</param>
        /// <returns>The object returned by the private method to call.</returns>
        internal static object GetNonPubicProperty(object instance, string propertyName)
        {
            Type type = instance.GetType();
            var return_object = type.InvokeMember(
                propertyName,
                BindingFlags.Instance | BindingFlags.GetProperty,
                null,
                instance,
                null);

            return return_object;
        }

        /// <summary>
        /// Call a non public property of an object.
        /// </summary>
        /// <param name="instance">Object whose method to call is private.</param>
        /// <param name="propertyName">Name of the property to call.</param>
        /// <param name="value">Value to pass to the porperty setter</param>
        /// <returns>The object returned by the private method to call.</returns>
        internal static object SetNonPubicProperty(object instance, string propertyName, object value)
        {
            Type type = instance.GetType();
            var parameters = new object[1]; // alwasy 1 because a property setter could only have one parameter
            var return_object = type.InvokeMember(
                propertyName,
                BindingFlags.Instance | BindingFlags.SetProperty,
                null,
                instance,
                parameters);

            return return_object;
        }

        /// <summary>
        /// Set the value of a non public field.
        /// </summary>
        /// <param name="instance">Object whose fild should be set.</param>
        /// <param name="fieldName">Name of the field to set.</param>
        /// <param name="value">Value to set the field with.</param>
        internal static void SetNonPublicField(object instance, string fieldName, object value)
        {
            Type type = instance.GetType();
            var parameters = new object[1]; // alwasy 1 because a property setter could only have one parameter
            parameters[0] = value;
            type.InvokeMember(
                fieldName,
                BindingFlags.NonPublic | BindingFlags.Instance | BindingFlags.SetField,
                null,
                instance,
                parameters);
        }
    }
}
