import logging
import os
import re
import sqlalchemy
import sqlalchemy.orm

from omnetpp.common.task import *

__sphinx_mock__ = True # ignore this module in documentation

_logger = logging.getLogger(__name__)

default_database = "sqlite:///omnetpp.sqlite?check_same_thread=False"

mapper_registry = sqlalchemy.orm.registry()

orm_mappers = {}

def add_orm_mapper(name, function):
    if not name in orm_mappers:
        orm_mappers[name] = function
    else:
        # NOTE: we assume that when a file is re-loaded the ORM mapper is the same
        pass

database_engine = None

def allocate_persistent_object(cls):
    object = cls.__new__(cls)
    super_class = super(cls, object) # find PersistentObject superclass
    # super_class = super(object.__class__.__mro__[-3], object) # find PersistentObject superclass
    super_class.__init__()
    object._sa_instance_state = sqlalchemy.orm.state.InstanceState(object, object._sa_class_manager)
    return object

def clone_persistent_object(object):
    clone = allocate_persistent_object(object.__class__)
    for name, value in object.__dict__.items():
        if not name.startswith("_") and not hasattr(value, '__dict__'):
            setattr(clone, name, value)
    return clone

def initialize_database_engine(database=default_database, clear=False):
    global database_engine, session_factory, Session
    database_filename = os.path.join(os.getcwd(), re.sub(r".*?:///([\w\.]+)\?.*", r"\1", database))
    if clear and os.path.exists(database_filename):
        os.remove(database_filename)
    database_engine = sqlalchemy.create_engine(database, future=True, execution_options={"isolation_level": "SERIALIZABLE"})
    for key, value in orm_mappers.items():
        value()
    mapper_registry.metadata.create_all(database_engine)
    session_factory = sqlalchemy.orm.sessionmaker(bind=database_engine)
    Session = sqlalchemy.orm.scoped_session(session_factory)

def call_with_database_session(function, **kwargs):
    with Session() as database_session:
        result = function(database_session=database_session, **kwargs)
        database_session.commit()
    return result
